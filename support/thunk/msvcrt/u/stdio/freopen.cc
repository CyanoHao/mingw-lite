#include <thunk/_common.h>
#include <thunk/string.h>

#include <errno.h>
#include <stdio.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(msvcrt,
                 0,
                 FILE *,
                 __cdecl,
                 freopen,
                 const char *path,
                 const char *mode,
                 FILE *stream)
  {
    if (!path || !mode) {
      _set_errno(EINVAL);
      return nullptr;
    }

    d::w_str w_path;
    if (!w_path.from_u(path)) {
      _set_errno(ENOMEM);
      return nullptr;
    }

    d::w_str w_mode;
    if (!w_mode.from_u(mode)) {
      _set_errno(ENOMEM);
      return nullptr;
    }

    return _wfreopen(w_path.c_str(), w_mode.c_str(), stream);
  }
} // namespace mingw_thunk
