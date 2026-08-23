#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // The whole string, which the reference spells as _mbsnset with an
  // unbounded count.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 unsigned char *,
                 __cdecl,
                 _mbsset,
                 unsigned char *dst,
                 unsigned int val)
  {
    if (!dst) {
      _set_errno(EINVAL);
      return nullptr;
    }
    bool dud = false;
    mbstring::set_chars(dst, val, (size_t)-1, (size_t)-1, &dud);
    if (dud)
      _set_errno(EINVAL);
    return dst;
  }
} // namespace mingw_thunk
