#include "console.h"

#include <io.h>
#include <thunk/unicode.h>

#include <stdlib.h>
#include <string.h>

#include <windows.h>

namespace mingw_thunk::ucrt
{

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

  console::guard::guard() noexcept
      : c(nullptr)
  {
  }

  console::guard::guard(guard &&other) noexcept
      : c(other.c)
  {
    other.c = nullptr;
  }

  console::guard &console::guard::operator=(guard &&other) noexcept
  {
    if (c)
      c->release();
    c = other.c;
    other.c = nullptr;
    return *this;
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

  int console::putc(unsigned char c) noexcept
  {
    auto guard = acquire_guard();
    return putc_nolock(c);
  }

  int console::putc(wchar_t c) noexcept
  {
    auto guard = acquire_guard();
    return putc_nolock(c);
  }

  int console::putc_nolock(unsigned char c) noexcept
  {
    tail[tail_len++] = c;

    while (tail_len > 0)
    {
      int const need = i::u8_dec_len(tail[0]);

      if (need == 0)
      {
        // invalid leading byte
        drop_tail_nolock(1);
        if (!emit_nolock(g::u16_rep))
          return EOF;
        continue;
      }

      if (tail_len < need)
        // partial
        break;

      switch (need)
      {
      case 1:
        if (!emit_nolock(tail[0]))
          return EOF;
        break;
      case 2:
        if (!i::u8_is_trail(tail[1]))
        {
          drop_tail_nolock(1);
          if (!emit_nolock(g::u16_rep))
            return EOF;
          continue;
        }
        if (!emit_nolock(i::u8_dec_2({tail[0], tail[1]})))
          return EOF;
        break;
      case 3:
        if (!i::u8_is_trail(tail[1]) || !i::u8_is_trail(tail[2]))
        {
          drop_tail_nolock(1);
          if (!emit_nolock(g::u16_rep))
            return EOF;
          continue;
        }
        if (!emit_nolock(i::u8_dec_3({tail[0], tail[1], tail[2]})))
          return EOF;
        break;
      case 4:
        if (!i::u8_is_trail(tail[1]) || !i::u8_is_trail(tail[2]) ||
            !i::u8_is_trail(tail[3]))
        {
          drop_tail_nolock(1);
          if (!emit_nolock(g::u16_rep))
            return EOF;
          continue;
        }
        if (!emit_nolock(i::u8_dec_4({tail[0], tail[1], tail[2], tail[3]})))
          return EOF;
        break;
      }

      drop_tail_nolock(need);
    };
    return c;
  }

  int console::putc_nolock(wchar_t c) noexcept
  {
    if (i::u16_is_high(c))
    {
      if (whigh != 0)
      {
        if (!emit_nolock(g::u16_rep))
          return WEOF;
      }
      whigh = c;
      return c;
    }

    if (i::u16_is_low(c))
    {
      if (!i::u16_is_high(whigh))
      {
        if (!emit_nolock(g::u16_rep))
          return WEOF;
        return c;
      }
      if (!emit_nolock(i::u16_dec_2({whigh, c})))
        return WEOF;
      whigh = 0;
      return c;
    }

    if (!emit_nolock(c))
      return WEOF;
    return c;
  }

  size_t console::write_nolock(void const *p, size_t n) noexcept
  {
    unsigned char const *s = static_cast<unsigned char const *>(p);

    for (size_t i = 0; i < n; ++i)
    {
      if (putc_nolock(s[i]) == EOF)
        return i;
    }

    return n;
  }

  void console::open(int fd) noexcept
  {
    auto guard = acquire_guard();
    bind_nolock(fd);
  }

  void console::close() noexcept
  {
    auto guard = acquire_guard();
    flush_nolock();
    reset_nolock();
  }

  void console::flush() noexcept
  {
    auto guard = acquire_guard();
    flush_nolock();
  }

  void console::flush_nolock() noexcept
  {
    if (wlen == 0)
      return;

    write_console(fh, wbuf, wlen);
    wlen = 0;
  }

  bool console::is_console(int fd) noexcept
  {
    if (!_isatty(fd))
      return false;

    HANDLE h = (HANDLE)_get_osfhandle(fd);
    DWORD mode;
    return h != INVALID_HANDLE_VALUE && GetConsoleMode(h, &mode);
  }

  bool console::is_console(FILE *fp) noexcept
  {
    return is_console(_fileno(fp));
  }

  console *console::get(int fd, bool allocate) noexcept
  {
    if (fd < 0)
      return nullptr;

    HANDLE h = (HANDLE)_get_osfhandle(fd);

    if (fd < 3)
    {
      console *const c = &standard_instance[fd];
      if (c->fh != h)
      {
        auto guard = c->acquire_guard();
        if (c->fh != h)
          c->bind_nolock(fd, h);
      }
      return c;
    }

    while (__atomic_test_and_set(&dynamic_lock, __ATOMIC_ACQUIRE))
      Sleep(0);

    struct guard
    {
      ~guard() noexcept
      {
        __atomic_clear(&dynamic_lock, __ATOMIC_RELEASE);
      }
    } guard;

    for (int i = 0; i < dynamic_size; ++i)
    {
      console *const c = dynamic_instance[i];
      if (!c)
        continue;

      if (c->fd == fd)
      {
        if (c->fh != h)
        {
          auto guard = c->acquire_guard();
          if (c->fh != h)
            c->bind_nolock(fd, h);
        }
        return c;
      }
    }

    if (!allocate)
      return nullptr;

    for (int i = 0; i < dynamic_size; ++i)
    {
      if (!dynamic_instance[i])
      {
        auto p = (console *)malloc(sizeof(console));
        if (!p)
          return nullptr;

        new (p) console(fd, h);
        dynamic_instance[i] = p;
        return p;
      }

      if (dynamic_instance[i]->fd < 0)
      {
        auto guard = dynamic_instance[i]->acquire_guard();
        dynamic_instance[i]->bind_nolock(fd, h);
        return dynamic_instance[i];
      }
    }

    int const old_size = dynamic_size;
    int const new_size = dynamic_size + dynamic_inc;
    auto arr =
        (console **)realloc(dynamic_instance, sizeof(console *) * new_size);
    if (!arr)
      return nullptr;

    for (int i = old_size; i < new_size; ++i)
      arr[i] = nullptr;

    dynamic_instance = arr;
    dynamic_size = new_size;

    auto p = (console *)malloc(sizeof(console));
    if (!p)
      return nullptr;

    new (p) console(fd, h);
    dynamic_instance[old_size] = p;
    return p;
  }

  console *console::get(FILE *fp, bool allocate) noexcept
  {
    if (!fp)
      return nullptr;
    return get(_fileno(fp), allocate);
  }

  void console::flush_all() noexcept
  {
    for (int fd = 1; fd < 3; ++fd)
    {
      auto &c = standard_instance[fd];
      auto guard = c.acquire_guard();
      if (c.fd > 0)
        c.flush_nolock();
    }

    while (__atomic_test_and_set(&dynamic_lock, __ATOMIC_ACQUIRE))
      Sleep(0);

    struct guard
    {
      ~guard() noexcept
      {
        __atomic_clear(&dynamic_lock, __ATOMIC_RELEASE);
      }
    } guard;

    for (int i = 0; i < dynamic_size; ++i)
    {
      auto *const c = dynamic_instance[i];
      if (!c)
        continue;

      auto guard = c->acquire_guard();
      if (c->fd > 0)
        c->flush_nolock();
    }
  }

  console console::standard_instance[3] = {};
  console **console::dynamic_instance = nullptr;
  int console::dynamic_size = 0;
  bool console::dynamic_lock = false;

  void *console::operator new(size_t, void *ptr) noexcept
  {
    return ptr;
  }

  void console::bind_nolock(int fd) noexcept
  {
    bind_nolock(fd, (HANDLE)_get_osfhandle(fd));
  }

  void console::bind_nolock(int fd, HANDLE fh) noexcept
  {
    this->fd = fd;
    this->fh = fh;
    tail_len = 0;
    wlen = 0;
  }

  void console::reset_nolock() noexcept
  {
    fd = -1;
    fh = INVALID_HANDLE_VALUE;
    tail_len = 0;
    wlen = 0;
  }

  bool console::emit_nolock(char32_t cp) noexcept
  {
    wchar_t units[3];
    int n = 0;

    bool const newline = cp == 0x0a;
    if (newline)
      units[n++] = L'\r';
    if (i::u16_enc_len(cp) == 1)
    {
      units[n++] = (wchar_t)cp;
    }
    else
    {
      auto s = i::u16_enc_2(cp);
      units[n++] = s.high;
      units[n++] = s.low;
    }

    if (wlen + n > wbuf_size) [[unlikely]]
    {
      flush_nolock();
      write_console(fh, units, n);
    }
    else
    {
      for (int i = 0; i < n; ++i)
        wbuf[wlen++] = units[i];

      if (wlen >= wbuf_flush_threshold || standard_stream_should_flush(fd, cp))
        flush_nolock();
    }

    return true;
  }

  void console::drop_tail_nolock(int n) noexcept
  {
    switch (n)
    {
    case 0:
      return;
    case 1:
      tail[0] = tail[1];
      tail[1] = tail[2];
      tail[2] = tail[3];
      tail_len -= 1;
      break;
    case 2:
      tail[0] = tail[2];
      tail[1] = tail[3];
      tail_len -= 2;
      break;
    case 3:
      tail[0] = tail[3];
      tail_len -= 3;
      break;
    default:
      tail_len = 0;
    }
  }

  void console::write_console(HANDLE fh, wchar_t const *buf, int n) noexcept
  {
    DWORD written = 0;
    WriteConsoleW(fh, buf, n, &written, nullptr);
  }

  bool console::is_stdout(int fd) noexcept
  {
    return fd == 1;
  }

  bool console::is_stderr(int fd) noexcept
  {
    return fd == 2;
  }

  bool console::standard_stream_should_flush(int fd, char32_t cp) noexcept
  {
    return (is_stdout(fd) && cp == '\n') || is_stderr(fd);
  }

  bool is_console(int fd) noexcept
  {
    return console::is_console(fd);
  }

  bool is_console(FILE *fp) noexcept
  {
    return console::is_console(fp);
  }

  void console_open(int fd) noexcept
  {
    if (!console::is_console(fd))
      return;

    console::get(fd, true);
  }

  void console_close(int fd) noexcept
  {
    console *const c = console::get(fd, false);
    if (c)
      c->close();
  }

  void __attribute__((used, destructor)) flush_on_exit() noexcept
  {
    console::flush_all();
  }

} // namespace mingw_thunk::ucrt
