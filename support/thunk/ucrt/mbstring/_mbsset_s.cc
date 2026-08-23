#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // The whole string, which the reference spells as _mbsnset_s with an
  // unbounded count.  Note that this face has no zero-count no-op in the
  // reference, so a null destination is EINVAL here rather than success.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 errno_t,
                 __cdecl,
                 _mbsset_s,
                 unsigned char *dst,
                 size_t size,
                 unsigned int val)
  {
    return mbstring::set_s(dst, size, val, (size_t)-1, false);
  }
} // namespace mingw_thunk
