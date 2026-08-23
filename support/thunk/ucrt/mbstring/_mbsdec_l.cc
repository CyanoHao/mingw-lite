#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // Locale argument ignored; the _l twin of _mbsdec.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 const unsigned char *,
                 __cdecl,
                 _mbsdec_l,
                 const unsigned char *string,
                 const unsigned char *current,
                 _locale_t locale)
  {
    (void)locale;
    return mbstring::mbs_dec(string, current);
  }
} // namespace mingw_thunk
