#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // Locale argument ignored; the _l twin of _mbslen.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 size_t,
                 __cdecl,
                 _mbslen_l,
                 const unsigned char *string,
                 _locale_t locale)
  {
    (void)locale;
    return mbstring::mbs_len(string);
  }
} // namespace mingw_thunk
