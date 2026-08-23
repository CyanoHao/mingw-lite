#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // Locale argument ignored; the _l twin of _mbsrchr.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 const unsigned char *,
                 __cdecl,
                 _mbsrchr_l,
                 const unsigned char *string,
                 unsigned int ch,
                 _locale_t locale)
  {
    (void)locale;
    return mbstring::mbs_find(string, ch, true);
  }
} // namespace mingw_thunk
