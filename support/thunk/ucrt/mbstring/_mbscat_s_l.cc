#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // Locale argument ignored; the _l twin of _mbscat_s.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 errno_t,
                 __cdecl,
                 _mbscat_s_l,
                 unsigned char *dst,
                 size_t size,
                 const unsigned char *src,
                 _locale_t locale)
  {
    (void)locale;
    return mbstring::cat_s(dst, size, src);
  }
} // namespace mingw_thunk
