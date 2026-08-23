#include <thunk/_common.h>
#include "cvt_render.h"

#include <stdlib.h>

namespace mingw_thunk
{
  // Wine-anchored matrix (plan-3 §3.7): r9 null buffer -> rc 22, errno
  // untouched; r10 cap == 0 -> rc 34, buffer untouched, errno 34;
  // precision >= cap -> rc 34, buffer cleared (probe2: "1.5"/6 dies at
  // cap <= 6, succeeds at 7); rendered string longer than cap - 1 -> rc
  // 34 cleared ("1.23457e+06" fails at cap 8/9).  Edge divergence: the
  // native E-format path demands extra slack above strlen + 1 (needs 15
  // for the cap 11..14 window) — we require exactly strlen + 1.
  __DEFINE_THUNK(api_ms_win_crt_convert_l1_1_0,
                 0,
                 errno_t,
                 __cdecl,
                 _gcvt_s,
                 char *buffer,
                 size_t buffer_count,
                 double value,
                 int precision)
  {
    return u8crt_convert::gcvt_core(buffer, buffer_count, value, precision);
  }
} // namespace mingw_thunk
