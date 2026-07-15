#include <thunk/_common.h>
#include <thunk/wntcrt/stat.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(msvcrt,
                 0,
                 int,
                 __cdecl,
                 stat32i64,
                 const char *path,
                 struct _stat32i64 *buffer)
  {
    return _stat32i64(path, buffer);
  }
} // namespace mingw_thunk
