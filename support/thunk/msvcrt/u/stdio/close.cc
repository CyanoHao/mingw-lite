#include <thunk/_common.h>

#include <io.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(msvcrt, 0, int, __cdecl, close, int fd)
  {
    return _close(fd);
  }
} // namespace mingw_thunk
