#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/u8crt/ucrt.h>

#include <io.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(api_ms_win_crt_stdio_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 _write,
                 int fd,
                 const void *buffer,
                 unsigned int count)
  {
    return ucrt::_write(fd, buffer, count);
  }
} // namespace mingw_thunk
