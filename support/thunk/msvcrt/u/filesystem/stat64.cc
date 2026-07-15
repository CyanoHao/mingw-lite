#include <thunk/_common.h>
#include <thunk/wntcrt/stat.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(
      msvcrt, 0, int, __cdecl, stat64, const char *path, struct stat64 *buffer)
  {
    return _stat64(path, (struct _stat64 *)buffer);
  }
} // namespace mingw_thunk
