#include <thunk/_crt.h>
#include <thunk/console.h>

#include <io.h>

namespace mingw_thunk
{
  __DEFINE_UCRT_THUNK(api_ms_win_crt_stdio_l1_1_0, int, _close, int fd)
  {
    if (internal::is_console(fd)) {
      auto &buffer = internal::fd_io_buffer::get(fd);
      buffer.flush_all(fd);
    }
    return get__close()(fd);
  }
} // namespace mingw_thunk
