#include <thunk/_common.h>
#include <thunk/string.h>

#include <direct.h>
#include <errno.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(msvcrt, 0, int, __cdecl, _chdir, const char *path)
  {
    if (!path) {
      _set_errno(EINVAL);
      return -1;
    }

    d::w_str w_path;
    if (!w_path.from_u(path)) {
      _set_errno(ENOMEM);
      return -1;
    }

    return _wchdir(w_path.c_str());
  }
} // namespace mingw_thunk
