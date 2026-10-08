#include "console.h"

#include "../inc/corecrt_internal_ptd_propagation.h"

#include <thunk/unicode.h>

#include <errno.h>
#include <io.h>
#include <string.h>

#include <windows.h>

namespace mingw_thunk::ucrt
{

  console::console() noexcept
      : console(-1, INVALID_HANDLE_VALUE)
  {
  }

  console::console(int fd) noexcept
      : console(fd, (HANDLE)_get_osfhandle(fd))
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

  console::guard::guard(console *c) noexcept
      : c(c)
  {
  }

  console::guard::~guard() noexcept
  {
    c->release();
  }

  void console::acquire() noexcept
  {
    while (__atomic_test_and_set(&lock, __ATOMIC_ACQUIRE))
      Sleep(0);
  }

  console::guard console::acquire_guard() noexcept
  {
    acquire();
    return guard(this);
  }

  void console::release() noexcept
  {
    __atomic_clear(&lock, __ATOMIC_RELEASE);
  }

  void console::open(int fd) noexcept
  {
    auto guard = acquire_guard();
    reset_nolock();
    this->fd = fd;
    this->fh = (HANDLE)_get_osfhandle(fd);
  }

  void console::close() noexcept
  {
    auto guard = acquire_guard();
    flush();
    reset_nolock();
  }

  void console::reset() noexcept
  {
    auto guard = acquire_guard();
    reset_nolock();
  }

  void console::reset_nolock() noexcept
  {
    fd = -1;
    fh = INVALID_HANDLE_VALUE;
    tail_len = 0;
    wlen = 0;
  }

  console console::standard_instance[3] = {{0}, {1}, {2}};
  console **console::dynamic_instance = nullptr;
  int console::dynamic_size = 0;
  bool console::dynamic_lock = false;

  console *console::get(int fd, bool allocate) noexcept
  {
    if (fd < 0)
      return nullptr;

    if (fd < 3)
    {
      auto c = &standard_instance[fd];
      if (c->fd != fd)
        c->open(fd);
    }

    __atomic_test_and_set(&dynamic_lock, __ATOMIC_ACQUIRE);
    struct lock_guard
    {
      ~lock_guard() noexcept
      {
        __atomic_clear(&dynamic_lock, __ATOMIC_RELEASE);
      }
    } guard;

    for (int i = 0; i < dynamic_size; ++i)
    {
      if (dynamic_instance[i] && dynamic_instance[i]->fd == fd)
        return dynamic_instance[i];
    }

    if (!allocate)
      return nullptr;

    for (int i = 0; i < dynamic_size; ++i)
    {
      if (!dynamic_instance[i])
      {
        auto c = (console *)malloc(sizeof(console));
        if (!c)
          return nullptr;

        new (c) console(fd);
        dynamic_instance[i] = c;
        return c;
      }

      if (dynamic_instance[i]->fd == -1)
      {
        dynamic_instance[i]->open(fd);
        return dynamic_instance[i];
      }
    }

    int old_size = dynamic_size;
    int new_size = dynamic_size + dynamic_inc;
    auto arr =
        (console **)realloc(dynamic_instance, new_size * sizeof(console *));
    if (!arr)
      return nullptr;

    for (int i = old_size; i < new_size; ++i)
      arr[i] = nullptr;

    dynamic_instance = arr;
    dynamic_size = new_size;

    auto c = (console *)malloc(sizeof(console));
    if (!c)
      return nullptr;

    new (c) console(fd);
    dynamic_instance[old_size] = c;
    return c;
  }

  console *console::get(FILE *fp, bool allocate) noexcept
  {
    return get(_fileno(fp), allocate);
  }

  void *console::operator new(size_t, void *ptr) noexcept
  {
    return ptr;
  }

  namespace
  {

    /* pool size matches utf8-musl's console_channel: 8 simultaneous
     * console fds beyond the static trio; further ones fail closed */
    constexpr int kPoolSlots = 8;

    enum
    {
      kFree = 0,
      kInit = 1,
      kReady = 2
    };

    struct console_utf8_slot
    {
      volatile long state; /* publication barrier for ch below */
      console ch;
    };

    console g_trio[3] = {
        {SRWLOCK_INIT, -1, INVALID_HANDLE_VALUE, {}, 0, {}, 0, 0},
        {SRWLOCK_INIT, -1, INVALID_HANDLE_VALUE, {}, 0, {}, 0, 0},
        {SRWLOCK_INIT, -1, INVALID_HANDLE_VALUE, {}, 0, {}, 0, 0},
    };

    console_utf8_slot g_pool[kPoolSlots];

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

    void wipe_locked(console *ch) noexcept
    {
      ch->tail_len = 0;
      ch->wlen = 0;
      ch->batch_depth = 0;
    }

    void drop_tail_locked(console *ch, int n) noexcept
    {
      ch->tail_len -= n;
      memmove(ch->tail, ch->tail + n, (size_t)ch->tail_len);
    }

    bool report_write_failure(__crt_cached_ptd_host &ptd,
                              DWORD os_error) noexcept
    {
      errno_t e;
      switch (os_error)
      {
      case ERROR_ACCESS_DENIED:  /* native _write maps both of these to */
      case ERROR_INVALID_HANDLE: /* EBADF rather than EACCES/other */
        e = EBADF;
        break;
      case ERROR_NOT_ENOUGH_MEMORY:
        e = ENOMEM;
        break;
      default:
        e = EACCES; /* _dosmaperr's fallback */
        break;
      }

      ptd.get_errno().set(e);
      ptd.get_doserrno().set(os_error);
      return false;
    }

    /* one WriteConsoleW per invocation; errors drop the buffer (the
     * native failed-flush shape), short writes keep the remainder and
     * report ENOSPC like native _write's zero/partial-write path */
    bool
    flush_locked(console *ch, HANDLE h, __crt_cached_ptd_host *ptd) noexcept
    {
      if (ch->wlen == 0)
        return true;

      DWORD written = 0;
      if (!WriteConsoleW(h, ch->wbuf, (DWORD)ch->wlen, &written, nullptr))
      {
        ch->wlen = 0;
        if (ptd)
          (void)report_write_failure(*ptd, GetLastError());
        return false;
      }

      if (written < (DWORD)ch->wlen)
      {
        if (written > 0)
          memmove(ch->wbuf,
                  ch->wbuf + written,
                  (size_t)(ch->wlen - (int)written) * sizeof(wchar_t));
        ch->wlen -= (int)written;
        if (ptd)
        {
          ptd->get_errno().set(ENOSPC);
          ptd->get_doserrno().set(0);
        }
        return false;
      }

      ch->wlen = 0;
      return true;
    }

    /* append one code point as UTF-16 (CRLF-expanding LF the way the
     * native text-mode writers do, so output stays correct even when
     * the console has ENABLE_PROCESSED_OUTPUT off) */
    bool emit_locked(console *ch,
                     HANDLE h,
                     char32_t cp,
                     __crt_cached_ptd_host &ptd) noexcept
    {
      wchar_t units[3];
      int n = 0;

      if (cp == 0x0a)
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

      if (ch->wlen + n > (int)(sizeof ch->wbuf / sizeof ch->wbuf[0]))
        if (!flush_locked(ch, h, &ptd))
          return false;

      for (int i = 0; i < n; ++i)
        ch->wbuf[ch->wlen++] = units[i];

      /* bare (unguarded) writers deliver every completed code point
       * immediately: one WriteConsoleW per character is the parity of
       * native fputc on a console (one lowio _write per character),
       * and a sequence emitted one fputc per byte reassembles in the
       * channel tail across calls exactly like the native _mbBuffer.
       * Guarded calls suppress this and flush once at end-of-call via
       * console_utf8_end. */
      if (ch->batch_depth == 0 && ch->wlen > 0)
        return flush_locked(ch, h, &ptd);

      return true;
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

    /* caller holds ch->lock; consumes the byte, parking or emitting
     * code points as they complete */
    console_utf8_result write_byte_locked(console *ch,
                                          HANDLE h,
                                          unsigned char c,
                                          __crt_cached_ptd_host &ptd) noexcept
    {
      ch->tail[ch->tail_len++] = c;

      while (ch->tail_len > 0)
      {
        int const need = i::u8_dec_len(ch->tail[0]);

        if (need == 0)
        {
          /* stray trail byte or reserved lead: one replacement,
           * resync on the next byte */
          drop_tail_locked(ch, 1);
          if (!emit_locked(ch, h, g::u16_rep, ptd))
            return console_utf8_result::error;
          continue;
        }

        if (ch->tail_len < need)
          break; /* park the partial sequence (native _mbBuffer) */

        bool trails_ok = true;
        for (int j = 1; j < need; ++j)
          if (!i::u8_is_trail(ch->tail[j]))
          {
            trails_ok = false;
            break;
          }

        char32_t const cp = decode_point(ch->tail, need);
        bool const ok = trails_ok && valid_point(need, cp);

        /* consume first so a failed emit can never overfill the park */
        drop_tail_locked(ch, ok ? need : 1);

        if (!emit_locked(ch, h, ok ? cp : g::u16_rep, ptd))
          return console_utf8_result::error;
      }

      return console_utf8_result::ok;
    }

    /* ---- fd resolution ------------------------------------------------ */

    /* fd 0/1/2: the static trio owns the slots forever (the FILEs are
     * process-static), but the handle snapshot tracks freopen/dup2:
     * on change the buffers are dropped and the console probe re-runs
     * once.  Fast path is two lock-free reads; any racing rebind loses
     * the race once and re-validates on the next byte. */
    console *trio_for_fd(int fd, HANDLE h) noexcept
    {
      console *ch = &g_trio[fd];

      if (ch->fh == h)
        return ch->fd == fd ? ch : nullptr;

      AcquireSRWLockExclusive(&ch->lock);
      if (ch->fh != h)
      {
        wipe_locked(ch);
        ch->fh = h;
        ch->fd = probe_console(h) ? fd : -1;
      }
      ReleaseSRWLockExclusive(&ch->lock);

      return ch->fd == fd ? ch : nullptr;
    }

    /* drop a published slot whose (fd, handle) went stale: fd closed
     * and its number reused for a different file.  Buffered data is
     * never flushed -- it belongs to a dead fd. */
    void
    reset_stale_slot(console_utf8_slot *slot, int fd, HANDLE stale) noexcept
    {
      console *ch = &slot->ch;
      AcquireSRWLockExclusive(&ch->lock);
      if (__atomic_load_n(&slot->state, __ATOMIC_ACQUIRE) == kReady &&
          ch->fd == fd && ch->fh == stale)
      {
        wipe_locked(ch);
        ch->fd = -1;
        ch->fh = INVALID_HANDLE_VALUE;
        __atomic_store_n(&slot->state, kFree, __ATOMIC_RELEASE);
      }
      ReleaseSRWLockExclusive(&ch->lock);
    }

    console *pool_for_fd(int fd, HANDLE h, bool *exhausted) noexcept
    {
      *exhausted = false;

      /* pass 1: published (fd, handle) hit */
      for (console_utf8_slot &slot : g_pool)
      {
        if (__atomic_load_n(&slot.state, __ATOMIC_ACQUIRE) != kReady)
          continue;
        if (slot.ch.fd == fd && slot.ch.fh == h)
          return &slot.ch;
      }

      /* pass 2: drop slots keyed to a recycled fd number */
      for (console_utf8_slot &slot : g_pool)
      {
        if (__atomic_load_n(&slot.state, __ATOMIC_ACQUIRE) != kReady)
          continue;
        if (slot.ch.fd == fd && slot.ch.fh != h)
          reset_stale_slot(&slot, fd, slot.ch.fh);
      }

      /* pass 3: bind a free slot; a non-console fd is never published
       * (every write just re-resolves it through the lock-free fast
       * path, so it costs no slot and no state) */
      console_utf8_slot *mine = nullptr;
      for (console_utf8_slot &slot : g_pool)
      {
        long expected = kFree;
        if (!__atomic_compare_exchange_n(&slot.state,
                                         &expected,
                                         kInit,
                                         false,
                                         __ATOMIC_ACQ_REL,
                                         __ATOMIC_ACQUIRE))
          continue;

        console *ch = &slot.ch;
        bool const console = probe_console(h);
        AcquireSRWLockExclusive(&ch->lock);
        wipe_locked(ch);
        ch->fh = h;
        ch->fd = console ? fd : -1;
        ReleaseSRWLockExclusive(&ch->lock);
        __atomic_store_n(
            &slot.state, console ? kReady : kFree, __ATOMIC_RELEASE);

        if (!console)
          return nullptr; /* not exhausted: genuinely not a console */

        mine = &slot;
        break;
      }

      if (!mine)
      {
        *exhausted = true;
        return nullptr;
      }

      /* pass 4: defer to a concurrent publisher of the same fd so a
       * channel is never duplicated (split tails would corrupt
       * partial sequences) */
      for (console_utf8_slot &slot : g_pool)
      {
        if (&slot == mine)
          continue;
        if (__atomic_load_n(&slot.state, __ATOMIC_ACQUIRE) != kReady)
          continue;
        if (slot.ch.fd == fd && slot.ch.fh == h)
        {
          reset_stale_slot(mine, fd, mine->ch.fh);
          return &slot.ch;
        }
      }

      return &mine->ch;
    }

    enum class resolve_kind
    {
      non_console,
      console,
      exhausted /* console, but the pool has no free slot */
    };

    resolve_kind resolve_fd(int fd, console **out) noexcept
    {
      *out = nullptr;
      if (fd < 0)
        return resolve_kind::non_console;

      HANDLE const h = os_handle_of(fd);
      if (fd < 3)
      {
        *out = trio_for_fd(fd, h);
        return *out ? resolve_kind::console : resolve_kind::non_console;
      }

      bool exhausted = false;
      *out = pool_for_fd(fd, h, &exhausted);
      if (*out)
        return resolve_kind::console;
      return exhausted ? resolve_kind::exhausted : resolve_kind::non_console;
    }

  } // namespace

  console_utf8_batch console_utf8_begin(FILE *stream) noexcept
  {
    if (!stream)
      return {};

    int const fd = _fileno(stream);

    console *ch = nullptr;
    if (resolve_fd(fd, &ch) != resolve_kind::console)
      /* not a console, or the pool is exhausted (every byte of the
       * call then re-resolves and fails closed -- the utf8-musl
       * "sink" degraded mode): no batch either way */
      return {fd, nullptr};

    AcquireSRWLockExclusive(&ch->lock);
    ++ch->batch_depth;
    ReleaseSRWLockExclusive(&ch->lock);

    return {fd, ch};
  }

  void console_utf8_end(console_utf8_batch const &batch,
                        __crt_cached_ptd_host &ptd) noexcept
  {
    console *const ch = batch.channel;
    if (!ch)
      return; /* not a console (or never resolved): nothing buffered */

    HANDLE const h = os_handle_of(batch.fd);

    AcquireSRWLockExclusive(&ch->lock);
    if (ch->fd == batch.fd && ch->fh == h)
    {
      if (ch->batch_depth > 0)
        --ch->batch_depth;
      if (ch->batch_depth == 0)
        (void)flush_locked(ch, h, &ptd);
    }
    ReleaseSRWLockExclusive(&ch->lock);
  }

  console_utf8_result console_utf8_write_byte(
      int fd, unsigned char c, __crt_cached_ptd_host &ptd) noexcept
  {
    /* Every call resolves the fd afresh -- the fast path is a handle
     * read plus two lock-free loads, and the console probe only runs
     * when the snapshot changed (freopen/dup2/fd recycle); there is
     * deliberately no call-scoped cached verdict, because bare fputc
     * writers emit UTF-8 sequences one byte per call.  Attempt 1
     * retries once after an fd recycle invalidated the channel. */
    for (int attempt = 0; attempt < 2; ++attempt)
    {
      console *ch = nullptr;
      switch (resolve_fd(fd, &ch))
      {
      case resolve_kind::non_console:
        return console_utf8_result::not_console;
      case resolve_kind::exhausted:
        ptd.get_errno().set(EBADF);
        ptd.get_doserrno().set(0);
        return console_utf8_result::error;
      case resolve_kind::console:
        break;
      }

      HANDLE const h = os_handle_of(fd);
      AcquireSRWLockExclusive(&ch->lock);
      if (ch->fd == fd && ch->fh == h)
      {
        console_utf8_result const result = write_byte_locked(ch, h, c, ptd);
        ReleaseSRWLockExclusive(&ch->lock);
        return result;
      }
      ReleaseSRWLockExclusive(&ch->lock);
      /* stale: the fd was recycled mid-call; fall through and look it
       * up afresh (once) */
    }

    return console_utf8_result::not_console;
  }

  void console_utf8_flush(int fd, __crt_cached_ptd_host &ptd) noexcept
  {
    console *ch = nullptr;
    if (resolve_fd(fd, &ch) != resolve_kind::console)
      return;

    AcquireSRWLockExclusive(&ch->lock);
    if (ch->fd == fd)
      (void)flush_locked(ch, ch->fh, &ptd);
    ReleaseSRWLockExclusive(&ch->lock);
  }

  void console_utf8_flush_all(__crt_cached_ptd_host &ptd) noexcept
  {
    for (int fd = 0; fd < 3; ++fd)
    {
      console *ch = &g_trio[fd];
      AcquireSRWLockExclusive(&ch->lock);
      (void)flush_locked(ch, ch->fh, &ptd);
      ReleaseSRWLockExclusive(&ch->lock);
    }

    for (console_utf8_slot &slot : g_pool)
    {
      if (__atomic_load_n(&slot.state, __ATOMIC_ACQUIRE) != kReady)
        continue;
      console *ch = &slot.ch;
      AcquireSRWLockExclusive(&ch->lock);
      if (ch->fd != -1)
        (void)flush_locked(ch, ch->fh, &ptd);
      ReleaseSRWLockExclusive(&ch->lock);
    }
  }

  void console_utf8_on_open(int fd) noexcept
  {
    if (fd < 3)
      return; /* the trio keys on handle snapshots, not fd reuse */

    for (console_utf8_slot &slot : g_pool)
    {
      if (__atomic_load_n(&slot.state, __ATOMIC_ACQUIRE) != kReady)
        continue;
      if (slot.ch.fd == fd)
        reset_stale_slot(&slot, fd, slot.ch.fh);
    }
  }

  void console_utf8_release(int fd) noexcept
  {
    if (fd < 3)
    {
      if (fd != 0)
      {
        console *ch = &g_trio[fd];
        AcquireSRWLockExclusive(&ch->lock);
        (void)flush_locked(ch, ch->fh, nullptr);
        ReleaseSRWLockExclusive(&ch->lock);
      }
      return;
    }

    HANDLE const h = os_handle_of(fd);
    for (console_utf8_slot &slot : g_pool)
    {
      if (__atomic_load_n(&slot.state, __ATOMIC_ACQUIRE) != kReady)
        continue;
      if (slot.ch.fd != fd)
        continue;

      console *ch = &slot.ch;
      AcquireSRWLockExclusive(&ch->lock);
      if (__atomic_load_n(&slot.state, __ATOMIC_ACQUIRE) == kReady &&
          ch->fd == fd)
      {
        if (ch->fh == h)
          (void)flush_locked(ch, h, nullptr); /* fd still live: flush */
        wipe_locked(ch);
        ch->fd = -1;
        ch->fh = INVALID_HANDLE_VALUE;
        __atomic_store_n(&slot.state, kFree, __ATOMIC_RELEASE);
      }
      ReleaseSRWLockExclusive(&ch->lock);
      return;
    }
  }

} // namespace mingw_thunk::ucrt
