#include <thunk/_common.h>

#include "mbs_pred.h"

namespace mingw_thunk
{
  // the CJK block row 1 over a decoded scalar value (plan-3 D7a/D8h).  Answers
  // 0/1 rather than a ctype mask (D8e).
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 _ismbcl0,
                 unsigned int c)
  {
    return mbstring::ismbc(c, mbstring::P_L0);
  }
} // namespace mingw_thunk
