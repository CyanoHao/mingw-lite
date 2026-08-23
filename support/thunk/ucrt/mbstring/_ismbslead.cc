#include <thunk/_common.h>

#include "mbs_pred.h"

namespace mingw_thunk
{
  // Is `current` the first byte of a well-formed character?  The
  // reference calls the test meaningless under UTF-8 and answers 0
  // everywhere; ours answers by UTF-8 context (plan-3 D8c).
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 _ismbslead,
                 const unsigned char *s,
                 const unsigned char *current)
  {
    return mbstring::ismbslead(s, current);
  }
} // namespace mingw_thunk
