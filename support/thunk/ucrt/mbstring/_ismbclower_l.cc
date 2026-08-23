#include <thunk/_common.h>

#include "mbs_pred.h"

namespace mingw_thunk
{
  // Locale argument ignored; the _l twin of _ismbclower.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 _ismbclower_l,
                 unsigned int c,
                 _locale_t locale)
  {
    (void)locale;
    return mbstring::ismbc(c, mbstring::P_LOWER);
  }
} // namespace mingw_thunk
