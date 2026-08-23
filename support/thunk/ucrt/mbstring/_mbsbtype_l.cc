#include <thunk/_common.h>

#include "mbs_pred.h"

namespace mingw_thunk
{
  // Locale argument ignored; the _l twin of _mbsbtype.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 _mbsbtype_l,
                 const unsigned char *s,
                 size_t pos,
                 _locale_t locale)
  {
    (void)locale;
    return mbstring::mbsbtype(s, pos);
  }
} // namespace mingw_thunk
