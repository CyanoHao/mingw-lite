#include <thunk/_common.h>

#include "mbs_pred.h"

namespace mingw_thunk
{
  // Locale argument ignored; the _l twin of _ismbslead.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 _ismbslead_l,
                 const unsigned char *s,
                 const unsigned char *current,
                 _locale_t locale)
  {
    (void)locale;
    return mbstring::ismbslead(s, current);
  }
} // namespace mingw_thunk
