#include <thunk/_common.h>
#include <thunk/string.h>
#include <thunk/u8crt/musl.h>

#include <errno.h>
#include <io.h>

namespace mingw_thunk
{
  // errno_t-shaped _sopen: *fd pre-set to -1 (wine anchor: -1 also on
  // the null-path error path), then the _wsopen wide native with the
  // transcode + console-channel fd-reuse guard shared with _sopen.
  // Failure returns the errno code directly (wine anchor: share
  // violation -> 13).
  __DEFINE_THUNK(api_ms_win_crt_stdio_l1_1_0,
                 0,
                 errno_t,
                 __cdecl,
                 _sopen_s,
                 int *fd,
                 const char *path,
                 int oflag,
                 int shflag,
                 int pmode)
  {
    if (!fd) {
      _set_errno(EINVAL);
      return EINVAL;
    }

    *fd = -1;

    if (!path) {
      _set_errno(EINVAL);
      return EINVAL;
    }

    d::w_str w_path;
    if (!w_path.from_u(path)) {
      _set_errno(ENOMEM);
      return ENOMEM;
    }

    int f = _wsopen(w_path.c_str(), oflag, shflag, pmode);
    if (f == -1) {
      int e = EINVAL;
      _get_errno(&e);
      return e;
    }

    *fd = f;
    musl::console_channel_on_open(f);
    return 0;
  }
} // namespace mingw_thunk
