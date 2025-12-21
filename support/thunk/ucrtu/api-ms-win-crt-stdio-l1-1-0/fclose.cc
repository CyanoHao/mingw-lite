#include <thunk/_crt.h>
#include <thunk/console.h>

#include <stdio.h>

namespace mingw_thunk
{
  __DEFINE_UCRT_THUNK(api_ms_win_crt_stdio_l1_1_0, int, fclose, FILE *stream)
  {
    int fd = _fileno(stream);
    if (internal::is_console(fd)) {
      auto &buffer = internal::fd_io_buffer::get(fd);
      buffer.flush_all(fd);
    }

    return get_fclose()(stream);
  }
} // namespace mingw_thunk
