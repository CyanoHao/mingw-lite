#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/u8crt/ucrt.h>

#include <errno.h>
#include <stdio.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(api_ms_win_crt_stdio_l1_1_0,
                 0,
                 FILE *,
                 __cdecl,
                 _fdopen,
                 int fd,
                 const char *mode)
  {
    if (!mode) {
      _set_errno(EINVAL);
      return nullptr;
    }

    FILE *fp = __ms__fdopen(fd, mode);
    if (!fp) {
      return fp;
    }

    ucrt::console_open(_fileno(fp));
    return fp;
  }
} // namespace mingw_thunk
