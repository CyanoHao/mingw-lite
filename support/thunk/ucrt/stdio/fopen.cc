#include <thunk/_common.h>
#include <thunk/string.h>
#include <thunk/u8crt/musl.h>

#include <errno.h>
#include <stdio.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(api_ms_win_crt_stdio_l1_1_0,
                 0,
                 FILE *,
                 __cdecl,
                 fopen,
                 const char *path,
                 const char *mode)
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

    FILE *fp = _wfopen(w_path.c_str(), w_mode.c_str());
    if (fp)
      musl::console_channel_on_open(_fileno(fp));
    return fp;
  }
} // namespace mingw_thunk
