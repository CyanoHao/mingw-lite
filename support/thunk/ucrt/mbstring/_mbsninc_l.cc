#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // Locale argument ignored; the _l twin of _mbsninc.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 const unsigned char *,
                 __cdecl,
                 _mbsninc_l,
                 const unsigned char *string,
                 size_t count,
                 _locale_t locale)
  {
    (void)locale;
    if (!string)
      return nullptr;
    return string + mbstring::mbs_nbcnt(string, count);
  }
} // namespace mingw_thunk
