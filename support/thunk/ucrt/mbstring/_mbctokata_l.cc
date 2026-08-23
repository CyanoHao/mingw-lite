#include <thunk/_common.h>

#include "mbs_char.h"

namespace mingw_thunk
{
  // Locale argument ignored; the _l twin of _mbctokata.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 unsigned int,
                 __cdecl,
                 _mbctokata_l,
                 unsigned int c,
                 _locale_t locale)
  {
    (void)locale;
    return mbstring::to_kata(c);
  }
} // namespace mingw_thunk
