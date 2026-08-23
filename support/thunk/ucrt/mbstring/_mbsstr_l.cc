#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // Locale argument ignored; the _l twin of _mbsstr.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 const unsigned char *,
                 __cdecl,
                 _mbsstr_l,
                 const unsigned char *string,
                 const unsigned char *sub,
                 _locale_t locale)
  {
    (void)locale;
    return mbstring::mbs_strstr(string, sub);
  }
} // namespace mingw_thunk
