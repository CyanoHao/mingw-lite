#include <thunk/_common.h>

#include "mbs_pred.h"

namespace mingw_thunk
{
  // Is `current` a continuation byte of one that started earlier?
  // See plan-3 D8c.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 _ismbstrail,
                 const unsigned char *s,
                 const unsigned char *current)
  {
    return mbstring::ismbstrail(s, current);
  }
} // namespace mingw_thunk
