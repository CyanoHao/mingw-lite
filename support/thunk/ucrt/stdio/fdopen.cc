#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/u8crt/musl.h>

#include "../../utf8-ucrt/mingw/console.h"

#include <errno.h>
#include <stdio.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(api_ms_win_crt_stdio_l1_1_0,
                 0,
                 FILE *,
                 __cdecl,
                 fdopen,
                 int fd,
                 const char *mode)
  {
    return _fdopen(fd, mode);
  }
} // namespace mingw_thunk
