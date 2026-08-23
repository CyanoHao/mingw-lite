#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // Locale argument ignored; the _l twin of _mbsupr_s.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 errno_t,
                 __cdecl,
                 _mbsupr_s_l,
                 unsigned char *string,
                 size_t size,
                 _locale_t locale)
  {
    (void)locale;
    return mbstring::case_s(string, size, 1);
  }
} // namespace mingw_thunk
