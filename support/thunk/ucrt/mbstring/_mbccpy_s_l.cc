#include <thunk/_common.h>

#include "mbs_char.h"

namespace mingw_thunk
{
  // Locale argument ignored; the _l twin of _mbccpy_s.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 errno_t,
                 __cdecl,
                 _mbccpy_s_l,
                 unsigned char *dst,
                 size_t size,
                 int *pcopied,
                 const unsigned char *src,
                 _locale_t locale)
  {
    (void)locale;
    return mbstring::ccpy_s(dst, size, pcopied, src);
  }
} // namespace mingw_thunk
