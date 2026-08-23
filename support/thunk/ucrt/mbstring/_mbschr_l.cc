#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // Locale argument ignored; the _l twin of _mbschr.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 const unsigned char *,
                 __cdecl,
                 _mbschr_l,
                 const unsigned char *string,
                 unsigned int ch,
                 _locale_t locale)
  {
    (void)locale;
    return mbstring::mbs_find(string, ch, false);
  }
} // namespace mingw_thunk
