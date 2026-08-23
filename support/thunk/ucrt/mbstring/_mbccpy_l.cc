#include <thunk/_common.h>

#include "mbs_char.h"

namespace mingw_thunk
{
  // Locale argument ignored; the _l twin of _mbccpy.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 void,
                 __cdecl,
                 _mbccpy_l,
                 unsigned char *dst,
                 const unsigned char *src,
                 _locale_t locale)
  {
    (void)locale;
    mbstring::ccpy(dst, src);
  }
} // namespace mingw_thunk
