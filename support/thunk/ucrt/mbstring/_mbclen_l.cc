#include <thunk/_common.h>

#include "mbs_char.h"

namespace mingw_thunk
{
  // Locale argument ignored; the _l twin of _mbclen.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 size_t,
                 __cdecl,
                 _mbclen_l,
                 const unsigned char *string,
                 _locale_t locale)
  {
    (void)locale;
    return mbstring::clen(string);
  }
} // namespace mingw_thunk
