#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // Locale argument ignored; the _l twin of _mbsnbicmp.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 _mbsnbicmp_l,
                 const unsigned char *s1,
                 const unsigned char *s2,
                 size_t count,
                 _locale_t locale)
  {
    (void)locale;
    return mbstring::mbs_compare(
        s1, s2, count, mbstring::CMP_FOLD, mbstring::BND_BYTES);
  }
} // namespace mingw_thunk
