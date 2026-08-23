#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/u8crt/ucrt.h>

#include <stdio.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(api_ms_win_crt_stdio_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 fputs,
                 const char *s,
                 FILE *stream)
  {
    return ucrt::fputs(s, stream);
  }
} // namespace mingw_thunk
