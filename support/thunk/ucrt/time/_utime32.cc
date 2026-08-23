#include <thunk/_common.h>
#include <thunk/string.h>

#include <errno.h>
#include <sys/utime.h>

namespace mingw_thunk
{
  // The time family's plain-wide delegation, the same shape as _utime64.
  // __utimbuf32 already carries __time32_t, so the caller's time_t32
  // values reach _wutime32 untouched: there is no 32<->64 fold here to
  // get wrong at the 2106 edge, and the native keeps its own validation.
  //
  // wine anchors: a null file name is -1/EINVAL, a missing file is
  // -1/ENOENT, a null `times` takes the current time, and a value past
  // 2038 (0xFFFFFFFF and 3000000000) is accepted rather than rejected --
  // the plain delegation reproduces all of them by construction.
  __DEFINE_THUNK(api_ms_win_crt_time_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 _utime32,
                 const char *path,
                 struct __utimbuf32 *times)
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

    return _wutime32(w_path.c_str(), times);
  }
} // namespace mingw_thunk
