#include <thunk/_common.h>

#include "mbs_pred.h"

namespace mingw_thunk
{
  // The byte-role split a DBCS lead/trail pair cannot express:
  // sequence-first versus continuation (plan-3 D8b).
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 _mbsbtype,
                 const unsigned char *s,
                 size_t pos)
  {
    return mbstring::mbsbtype(s, pos);
  }
} // namespace mingw_thunk
