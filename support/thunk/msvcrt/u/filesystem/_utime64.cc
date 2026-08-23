#include <thunk/_common.h>
#include <thunk/string.h>

#include <errno.h>
#include <sys/utime.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(msvcrt,
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

  // upstream added this for msvcrt32 (< NT 5.0)
  __DECLARE_FORCE_OVERRIDE_MINGW_EMU(_utime64)
} // namespace mingw_thunk
