#include <thunk/_common.h>
#include <thunk/string.h>

#include <errno.h>
#include <sys/utime.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(api_ms_win_crt_time_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 _utime64,
                 const char *path,
                 struct __utimbuf64 *times)
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

    return _wutime64(w_path.c_str(), times);
  }
} // namespace mingw_thunk
