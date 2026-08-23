#include <thunk/_common.h>
#include "cvt_render.h"

#include <errno.h>
#include <stdlib.h>

namespace mingw_thunk
{
  // Wine-anchored matrix: digits+1 succeeds ("100" at cap 4 for 1.005/2);
  // the probed cap-3 case kills the wine shell (invalid-parameter bug —
  // graceful ERANGE here, divergence note in cvt_render.h); r13 null
  // buffer -> rc 22 graceful.
  __DEFINE_THUNK(api_ms_win_crt_convert_l1_1_0,
                 0,
                 errno_t,
                 __cdecl,
                 _fcvt_s,
                 char *buffer,
                 size_t buffer_count,
                 double value,
                 int count,
                 int *decpt,
                 int *sign)
  {
    return u8crt_convert::fcvt_core(buffer, buffer_count, value, count, decpt, sign);
  }
} // namespace mingw_thunk
