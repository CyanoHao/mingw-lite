#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // The first min(count, length) characters.  The reference has no
  // validation section at all, so the null guard here is our own
  // hardening -- a native null deref is not a behaviour worth copying.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 size_t,
                 __cdecl,
                 _mbsnlen,
                 const unsigned char *string,
                 size_t count)
  {
    return mbstring::mbs_nlen(string, count);
  }
} // namespace mingw_thunk
