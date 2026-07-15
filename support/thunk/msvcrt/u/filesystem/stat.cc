#include <thunk/_common.h>
#include <thunk/wntcrt/stat.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(
      msvcrt, 0, int, __cdecl, stat, const char *path, struct stat *buffer)
  {
#ifdef _WIN64
    return _stat32(path, (struct _stat32 *)buffer);
#else
    return _stat64i32(path, (struct _stat64i32 *)buffer);
#endif
  }
} // namespace mingw_thunk
