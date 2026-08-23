#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // Locale argument ignored; the _l twin of _mbsrev.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 unsigned char *,
                 __cdecl,
                 _mbsrev_l,
                 unsigned char *string,
                 _locale_t locale)
  {
    (void)locale;
    return mbstring::mbs_rev(string);
  }
} // namespace mingw_thunk
