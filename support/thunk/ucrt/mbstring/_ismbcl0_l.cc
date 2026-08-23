#include <thunk/_common.h>

#include "mbs_pred.h"

namespace mingw_thunk
{
  // Locale argument ignored; the _l twin of _ismbcl0.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 _ismbcl0_l,
                 unsigned int c,
                 _locale_t locale)
  {
    (void)locale;
    return mbstring::ismbc(c, mbstring::P_L0);
  }
} // namespace mingw_thunk
