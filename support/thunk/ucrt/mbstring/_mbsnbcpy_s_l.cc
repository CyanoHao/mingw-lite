#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // Locale argument ignored; the _l twin of _mbsnbcpy_s.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 errno_t,
                 __cdecl,
                 _mbsnbcpy_s_l,
                 unsigned char *dst,
                 size_t size,
                 const unsigned char *src,
                 size_t count,
                 _locale_t locale)
  {
    (void)locale;
    return mbstring::ncopy_s(dst, size, src, count, true);
  }
} // namespace mingw_thunk
