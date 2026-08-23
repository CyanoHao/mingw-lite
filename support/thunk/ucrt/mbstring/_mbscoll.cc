#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // Collation order is code-point order, mirroring M8's
  // strcoll -> strcmp decision (plan-3 5.1).
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 _mbscoll,
                 const unsigned char *s1,
                 const unsigned char *s2)
  {
    return mbstring::mbs_compare(
        s1, s2, 0, mbstring::CMP_RAW, mbstring::BND_STR);
  }
} // namespace mingw_thunk
