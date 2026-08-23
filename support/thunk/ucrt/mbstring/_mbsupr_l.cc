#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // Locale argument ignored; the _l twin of _mbsupr.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 unsigned char *,
                 __cdecl,
                 _mbsupr_l,
                 unsigned char *string,
                 _locale_t locale)
  {
    (void)locale;
    return mbstring::case_s(string, (size_t)-1, 1) == 0 ? string : nullptr;
  }
} // namespace mingw_thunk
