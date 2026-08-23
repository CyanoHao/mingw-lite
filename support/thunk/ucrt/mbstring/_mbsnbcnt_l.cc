#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // Locale argument ignored; the _l twin of _mbsnbcnt.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 size_t,
                 __cdecl,
                 _mbsnbcnt_l,
                 const unsigned char *string,
                 size_t char_count,
                 _locale_t locale)
  {
    (void)locale;
    return mbstring::mbs_nbcnt(string, char_count);
  }
} // namespace mingw_thunk
