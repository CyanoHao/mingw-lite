#include "console.h"

#include <thunk/unicode.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <windows.h>

// Design (native references are to the 10.0.22621 UCRT sources):
//
//   * Byte-by-byte reassembly.  The engine's stream_output_adapter
//     writes one character per _fputc_nolock_internal call, and a bare
//     fputc loop is one stdio call per byte, so this module mirrors
//     ucrtbase's write_double_translated_ansi_nolock (lowio/write.cpp):
//     a lead byte without its trail bytes is parked per file (native
//     parks it in the lowio _mbBuffer) and "written" is reported for
//     it; the code point is completed by whichever write supplies the
//     missing bytes -- across stdio calls, because the tail lives in
//     the per-(fd, handle) object, not in any per-call state.
//
//   * Buffering policy (native stdio doctrine): stderr is unbuffered
//     on a console (one WriteConsoleW per completed character), stdout
//     is line-buffered (flush at LF), every other console is fully
//     buffered until the wbuf fills or an upper layer flushes
//     explicitly -- the ported __acrt_stdio_temporary_buffering_guard
//     flushes stdout/stderr once at end of each stdio call, fflush and
//     close do so through the hooks, and a stdin read flushes them the
//     way the classic CRT does before console input.
//
//   * "Always check console first": every get()/open()/put() compares
//     the stored handle snapshot against a fresh _get_osfhandle(fd)
//     and re-probes GetConsoleMode only on mismatch.  A non-console fd
//     resolves to a negative-cache object (fd == -1, snapshot kept),
//     never to nullptr: nullptr from get(fd, true) means allocation
//     failure only, and the caller fails closed (EOF + ENOMEM) so
//     UTF-8 output can never leak onto the native ANSI path.
//
//   * Concurrency: one one-byte spinlock per object (lock cmpxchg via
//     __atomic_test_and_set; Sleep(0) yields everywhere from 9x on --
//     SRWLOCK would need Vista) plus one spinlock for the dynamic
//     registry.  Lock order is registry -> object, never reversed.
//     The stdio engine holds the object lock for a whole stdio call
//     through the ptd-installed console pointer, so a guarded call
//     pays one acquire and one WriteConsoleW.

namespace mingw_thunk::ucrt
{

  namespace
  {

    HANDLE os_handle_of(int fd) noexcept
    {
      intptr_t const h = _get_osfhandle(fd);
      return h == -1 ? INVALID_HANDLE_VALUE : (HANDLE)h;
    }

    bool probe_console(HANDLE h) noexcept
    {
      DWORD mode;
      return h != INVALID_HANDLE_VALUE && GetConsoleMode(h, &mode) != FALSE;
    }

    char32_t decode_point(unsigned char const *t, int need) noexcept
    {
      unsigned const mask = need == 1   ? 0x7fu
                            : need == 2 ? 0x1fu
                            : need == 3 ? 0x0fu
                                        : 0x07u;
      char32_t cp = t[0] & mask;
      for (int j = 1; j < need; ++j)
        cp = (cp << 6) | (unsigned)(t[j] & 0x3fu);
      return cp;
    }

    bool valid_point(int need, char32_t cp) noexcept
    {
      switch (need)
      {
      case 1:
        return true;
      case 2:
        return cp >= 0x80; /* reject overlong */
      case 3:
        return cp >= 0x800 && !(cp >= 0xd800 && cp < 0xe000);
      case 4:
        return cp >= 0x10000 && cp <= 0x10ffff;
      }
      return false;
    }

    void report_write_failure(DWORD os_error) noexcept
    {
      switch (os_error)
      {
      case ERROR_ACCESS_DENIED:  /* native _write maps both of these */
      case ERROR_INVALID_HANDLE: /* to EBADF rather than EACCES      */
        errno = EBADF;
        break;
      case ERROR_NOT_ENOUGH_MEMORY:
        errno = ENOMEM;
        break;
      default:
        errno = EACCES; /* _dosmaperr's fallback */
        break;
      }
    }

  } // namespace

  console::console() noexcept
      : console(-1, INVALID_HANDLE_VALUE)
  {
  }

  console::console(int fd, HANDLE fh) noexcept
      : lock(0)
      , fd(fd)
      , fh(fh)
      , tail_len(0)
      , wlen(0)
  {
  }

  void console::acquire() noexcept
  {
    /* one-byte test-and-set spinlock: the locked cmpxchg behind
     * __atomic_test_and_set is as old as the CPU itself, and Sleep(0)
     * yields on every Windows including 9x. */
    while (__atomic_test_and_set(&lock, __ATOMIC_ACQUIRE))
      Sleep(0);
  }

  void console::release() noexcept
  {
    __atomic_clear(&lock, __ATOMIC_RELEASE);
  }

  console::guard console::acquire_guard() noexcept
  {
    acquire();
    return guard(this);
  }

  console::guard::~guard() noexcept
  {
    if (c)
      c->release();
  }

  console::guard::guard(console *c) noexcept
      : c(c)
  {
  }

  /* lock held: drop whatever belonged to the previous binding -- never
   * flushed, it belongs to a dead fd -- snapshot h, and record the
   * verdict made right now ("always check console first"). */
  void console::bind_nolock(int fd, HANDLE h, bool is_console) noexcept
  {
    tail_len = 0;
    wlen = 0;
    fh = h;
    this->fd = is_console ? fd : -1;
  }

  void console::open(int fd) noexcept
  {
    HANDLE const h = os_handle_of(fd);
    acquire();
    bind_nolock(fd, h, probe_console(h));
    release();
  }

  void console::close() noexcept
  {
    acquire();
    flush_nolock(); /* validates internally: a recycled fd drops its
                     * buffers silently instead of writing them */
    reset_nolock();
    release();
  }

  void console::reset() noexcept
  {
    acquire();
    reset_nolock();
    release();
  }

  void console::reset_nolock() noexcept
  {
    fd = -1;
    fh = INVALID_HANDLE_VALUE;
    tail_len = 0;
    wlen = 0;
  }

  /* one WriteConsoleW per invocation; a failed write drops the buffer
   * (the native failed-flush shape), a short write keeps the remainder
   * and reports ENOSPC like native _write's zero/partial-write path */
  bool console::write_nolock() noexcept
  {
    if (wlen == 0)
      return true;

    DWORD written = 0;
    if (!WriteConsoleW(fh, wbuf, (DWORD)wlen, &written, nullptr))
    {
      wlen = 0;
      report_write_failure(GetLastError());
      return false;
    }

    if (written < (DWORD)wlen)
    {
      if (written > 0)
        memmove(wbuf,
                wbuf + written,
                (size_t)(wlen - (int)written) * sizeof(wchar_t));
      wlen -= (int)written;
      errno = ENOSPC;
      return false;
    }

    wlen = 0;
    return true;
  }

  /* lock held: write the pending UTF-16 out, but only if the fd is
   * still the one this object was bound to -- bytes buffered for a
   * recycled fd belong to a dead file and are dropped unflushed */
  void console::flush_nolock() noexcept
  {
    if (fd < 0 || wlen == 0)
      return;

    HANDLE const h = os_handle_of(fd);
    if (fh != h)
    {
      wlen = 0;
      return;
    }

    (void)write_nolock();
  }

  void console::flush() noexcept
  {
    acquire();
    flush_nolock();
    release();
  }

  void console::flush_stdout_or_stderr_nolock() noexcept
  {
    if (fd == 1 || fd == 2)
      flush_nolock();
  }

  void console::flush_stdout_or_stderr() noexcept
  {
    if (fd == 1 || fd == 2)
      flush();
  }

  /* append one code point as UTF-16, CRLF-expanding LF the way the
   * native text-mode writers do (correct even when the console has
   * ENABLE_PROCESSED_OUTPUT off), then apply the buffering policy */
  bool console::emit_nolock(char32_t cp) noexcept
  {
    wchar_t units[3];
    int n = 0;

    bool const newline = cp == 0x0a;
    if (newline)
      units[n++] = L'\r';
    if (cp < 0x10000)
    {
      units[n++] = (wchar_t)cp;
    }
    else
    {
      d::u16_2 const s = i::u16_enc_2(cp);
      units[n++] = s.high;
      units[n++] = s.low;
    }

    if (wlen + n > (int)(sizeof wbuf / sizeof wbuf[0]))
      if (!write_nolock())
        return false;

    for (int i = 0; i < n; ++i)
      wbuf[wlen++] = units[i];

    /* stderr: unbuffered (one write per completed character -- the
     * parity of native fputc on a console); stdout: line-buffered;
     * anything else: fully buffered until full or explicitly flushed */
    if (fd == 2 || (fd == 1 && newline))
      return write_nolock();

    return true;
  }

  /* lock held: consume the byte, parking or emitting code points as
   * they complete -- the moral equivalent of native lowio's _mbBuffer
   * translation loop (write_double_translated_ansi_nolock) */
  console::result console::put_nolock(unsigned char c) noexcept
  {
    tail[tail_len++] = c;

    while (tail_len > 0)
    {
      int const need = i::u8_dec_len(tail[0]);

      if (need == 0)
      {
        /* stray trail byte or reserved lead: one replacement, resync
         * on the next byte */
        drop_tail_nolock(1);
        if (!emit_nolock(g::u16_rep))
          return result::error;
        continue;
      }

      if (tail_len < need)
        break; /* park the partial sequence (native _mbBuffer) */

      bool trails_ok = true;
      for (int j = 1; j < need; ++j)
        if (!i::u8_is_trail(tail[j]))
        {
          trails_ok = false;
          break;
        }

      char32_t const cp = decode_point(tail, need);
      bool const ok = trails_ok && valid_point(need, cp);

      /* consume first so a failed emit can never overfill the park */
      drop_tail_nolock(ok ? need : 1);

      if (!emit_nolock(ok ? cp : g::u16_rep))
        return result::error;
    }

    return result::ok;
  }

  void console::drop_tail_nolock(int n) noexcept
  {
    tail_len -= n;
    memmove(tail, tail + n, (size_t)tail_len);
  }

  console::result console::put(int fd, unsigned char c) noexcept
  {
    /* lock held (the buffering guard or a bare bracket) */
    HANDLE const h = os_handle_of(fd);

    if (fh != h)
      /* the snapshot moved under us (freopen / dup2 / close+recycle):
       * re-make the console verdict at the fresh snapshot, dropping
       * whatever the dead binding had parked */
      bind_nolock(fd, h, probe_console(h));

    if (this->fd < 0)
      return result::not_console;

    return put_nolock(c);
  }

  console *console::get(int fd, bool allocate) noexcept
  {
    if (fd < 0)
      return nullptr;

    HANDLE const h = os_handle_of(fd);

    struct registry_guard
    {
      ~registry_guard() noexcept
      {
        __atomic_clear(&dynamic_lock, __ATOMIC_RELEASE);
      }
    };

    if (fd < 3)
    {
      /* the static trio is keyed by index: the slot exists whatever
       * the fd's identity is, and the handle snapshot tracks
       * freopen/dup2 -- on change the verdict is re-made exactly
       * once and then caches (including the negative verdict for a
       * redirected standard stream) */
      console *const c = &standard_instance[fd];
      if (c->fh != h)
      {
        c->acquire();
        if (c->fh != h)
          c->bind_nolock(fd, h, probe_console(h));
        c->release();
      }
      return c;
    }

    /* the registry lock guards only the pointer array and slot
     * life-cycle; entry contents stay consistent through the object
     * lock plus the per-use snapshot re-validation in put()/flush().
     * Lock order is registry -> object, never reversed. */
    {
      __atomic_test_and_set(&dynamic_lock, __ATOMIC_ACQUIRE);
      registry_guard unlock{};

      for (int i = 0; i < dynamic_size; ++i)
      {
        console *const c = dynamic_instance[i];
        if (!c)
          continue;

        if (c->fd == fd)
        {
          if (c->fh != h)
          {
            /* the fd number was recycled under us: rebind in place */
            c->acquire();
            if (c->fh != h)
              c->bind_nolock(fd, h, probe_console(h));
            c->release();
          }
          return c;
        }

        if (c->fd == -1 && c->fh == h)
          /* cached "not a console" for exactly this snapshot: saves
           * the probe while the fd stays a file */
          return c;
      }
    }

    if (!allocate)
      return nullptr; /* plain lookup miss (hooks); not an error */

    /* probe outside the registry lock so the syscall never extends
     * the critical section; a racing publisher just publishes the
     * same verdict twice */
    bool const con = probe_console(h);

    __atomic_test_and_set(&dynamic_lock, __ATOMIC_ACQUIRE);
    registry_guard unlock{};

    /* someone published while we probed: defer to it -- two objects
     * for one fd would split parked partial sequences */
    for (int i = 0; i < dynamic_size; ++i)
      if (dynamic_instance[i] && dynamic_instance[i]->fd == fd)
        return dynamic_instance[i];

    /* reuse an inert slot (free, or a negative cache for some other
     * snapshot) before growing the registry */
    console **slot = nullptr;
    for (int i = 0; i < dynamic_size; ++i)
      if (!dynamic_instance[i] || dynamic_instance[i]->fd == -1)
      {
        slot = &dynamic_instance[i];
        break;
      }

    if (!slot)
    {
      int const old_size = dynamic_size;
      int const new_size = old_size + dynamic_inc;
      auto const arr = (console **)realloc(
          dynamic_instance, (size_t)new_size * sizeof(console *));
      if (!arr)
        return nullptr; /* OOM: the only nullptr get() ever means */

      for (int i = old_size; i < new_size; ++i)
        arr[i] = nullptr;

      dynamic_instance = arr;
      dynamic_size = new_size;
      slot = &arr[old_size];
    }

    if (*slot)
    {
      /* rebind an inert object in place: it may still be referenced
       * by a racing put(), so go through the object lock */
      console *const c = *slot;
      c->acquire();
      c->bind_nolock(fd, h, con);
      c->release();
      return c;
    }

    auto const c = (console *)malloc(sizeof(console));
    if (!c)
      return nullptr; /* OOM */

    new (c) console(con ? fd : -1, h);
    *slot = c;
    return c;
  }

  console *console::get(FILE *fp, bool allocate) noexcept
  {
    if (!fp)
      return nullptr;
    return get(_fileno(fp), allocate);
  }

  console console::standard_instance[3] = {};
  console **console::dynamic_instance = nullptr;
  int console::dynamic_size = 0;
  bool console::dynamic_lock = false;

  void *console::operator new(size_t, void *ptr) noexcept
  {
    return ptr;
  }

} // namespace mingw_thunk::ucrt
