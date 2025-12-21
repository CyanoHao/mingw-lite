#include <thunk/_crt.h>
#include <thunk/console.h>
#include <thunk/string.h>
#include <thunk/unicode.h>

#include <stdio.h>

namespace mingw_thunk
{
  __DEFINE_UCRT_THUNK(api_ms_win_crt_stdio_l1_1_0, int, puts, const char *s)
  {
    int fd = _fileno(stdout);
    if (!internal::is_console(fd))
      return get_puts()(s);

    size_t len = strlen(s);
    auto &buffer = internal::fd_io_buffer::get(fd);
    buffer.append(s, len);
    buffer.push('\n');
    buffer.flush_all(fd);
    return len + 1;
  }
} // namespace mingw_thunk
