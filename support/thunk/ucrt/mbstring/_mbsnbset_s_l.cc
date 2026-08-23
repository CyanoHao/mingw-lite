#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // Locale argument ignored; the _l twin of _mbsnbset_s.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 errno_t,
                 __cdecl,
                 _mbsnbset_s_l,
                 unsigned char *dst,
                 size_t size,
                 unsigned int val,
                 size_t count,
                 _locale_t locale)
  {
    (void)locale;
    return mbstring::set_s(dst, size, val, count, true);
  }
} // namespace mingw_thunk
