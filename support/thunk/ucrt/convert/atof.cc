#include <thunk/_common.h>

#include <errno.h>
#include <stdlib.h>

namespace mingw_thunk
{
  // atof == strtod without endptr (reference atof.cpp shape).  Native
  // anchor r11: null string -> 0 + errno EINVAL (narrow strtod's native
  // crash shape is not mirrored — graceful family shape, see strtod.cc).
  __DEFINE_THUNK(api_ms_win_crt_convert_l1_1_0,
                 0,
                 double,
                 __cdecl,
                 atof,
                 const char *string)
  {
    return strtod(string, nullptr);
  }
} // namespace mingw_thunk
