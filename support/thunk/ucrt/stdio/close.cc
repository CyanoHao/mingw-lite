#include <thunk/_common.h>

#include <io.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(
      api_ms_win_crt_stdio_l1_1_0, 0, int, __cdecl, close, int fd)
  {
    return _close(fd);
  }
} // namespace mingw_thunk
