#include <thunk/_common.h>
#include "cvt_render.h"

#include <stdlib.h>

namespace mingw_thunk
{
  // Deprecated static-buffer face (not thread-safe by contract; the
  // native uses a per-thread buffer — compatibility note, api-set
  // §3.1b).  Requested digits are capped to the native _CVTBUFSIZE
  // margin (80 - 2, reference fcvt.cpp shape).  Wine anchors: 123.456/5
  // -> "12346"/3/0; 0.999/5 -> "99900"/0/0; 9.999/2 -> "10"/2/0;
  // 123.456/0 -> ""/3/0; 0/5 -> "00000"/0/0; -0/5 -> sign 1.
  __DEFINE_THUNK(api_ms_win_crt_convert_l1_1_0,
                 0,
                 char *,
                 __cdecl,
                 _ecvt,
                 double value,
                 int count,
                 int *decpt,
                 int *sign)
  {
    if (!decpt || !sign)
      return nullptr;

    // reference try_get_ptd_buffer analog: one process-wide slot
    static char buffer[80];

    int local_dec = 0;
    int local_sign = 0;
    const int capped = count > 78 ? 78 : count;
    if (u8crt_convert::ecvt_core(buffer, sizeof(buffer), value, capped, &local_dec,
                  &local_sign) != 0)
      return nullptr;

    *decpt = local_dec;
    *sign = local_sign;
    return buffer;
  }
} // namespace mingw_thunk
