#include <thunk/_common.h>
#include "cvt_render.h"

#include <errno.h>
#include <stdlib.h>

namespace mingw_thunk
{
  // Wine-anchored matrix (plan-3 §3.7): cap == count + 2 succeeds
  // ("12346" needs 7 for count 5); below that rc 34 + errno 34 with
  // buffer/decpt/sign untouched; null buffer / null out-params -> rc 22,
  // errno untouched; cap == 0 -> rc 34.
  __DEFINE_THUNK(api_ms_win_crt_convert_l1_1_0,
                 0,
                 errno_t,
                 __cdecl,
                 _ecvt_s,
                 char *buffer,
                 size_t buffer_count,
                 double value,
                 int count,
                 int *decpt,
                 int *sign)
  {
    return u8crt_convert::ecvt_core(buffer, buffer_count, value, count, decpt, sign);
  }
} // namespace mingw_thunk
