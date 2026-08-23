#include <thunk/_common.h>
#include <thunk/string.h>
#include <thunk/u8crt/ucrt.h>

#include <errno.h>
#include <stdio.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(api_ms_win_crt_stdio_l1_1_0,
                 0,
                 FILE *,
                 __cdecl,
                 _fsopen,
                 const char *path,
                 const char *mode,
                 int shflag)
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

    FILE *fp = _wfsopen(w_path.c_str(), w_mode.c_str(), shflag);
    if (fp) {
      ucrt::console_open(_fileno(fp));
    }
    return fp;
  }
} // namespace mingw_thunk
