#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // Locale argument ignored; the _l twin of _mbsnbset.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 unsigned char *,
                 __cdecl,
                 _mbsnbset_l,
                 unsigned char *dst,
                 unsigned int val,
                 size_t count,
                 _locale_t locale)
  {
    (void)locale;
    if (!count)
      return dst;
    if (!dst) {
      _set_errno(EINVAL);
      return nullptr;
    }
    bool dud = false;
    mbstring::set_chars(dst, val, (size_t)-1, count, &dud);
    if (dud)
      _set_errno(EINVAL);
    return dst;
  }
} // namespace mingw_thunk
