#include <thunk/_common.h>
#include "cvt_render.h"

#include <stdlib.h>

namespace mingw_thunk
{
  // Deprecated static-buffer face (see _ecvt.cc).  Wine anchors:
  // 123.456/2 -> "12346"/3; 1.005/2 -> "100"/1; 0.005/2 -> "1"/-1;
  // 1e-8/10 -> "100"/-7; 0/3 -> "000"/0; -0/2 -> "00"/0 sign 1;
  // 123.456/0 -> "123"/3; 1e20/3 -> 24 digits / 21.
  __DEFINE_THUNK(api_ms_win_crt_convert_l1_1_0,
                 0,
                 char *,
                 __cdecl,
                 _fcvt,
                 double value,
                 int count,
                 int *decpt,
                 int *sign)
  {
    if (!decpt || !sign)
      return nullptr;

    static char buffer[512];

    int local_dec = 0;
    int local_sign = 0;
    if (u8crt_convert::fcvt_core(buffer, sizeof(buffer), value, count, &local_dec,
                  &local_sign) != 0)
      return nullptr;

    *decpt = local_dec;
    *sign = local_sign;
    return buffer;
  }
} // namespace mingw_thunk
