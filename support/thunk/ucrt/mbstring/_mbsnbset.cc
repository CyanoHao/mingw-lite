#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // Overwrite the first count *bytes* with val, never leaving half a
  // sequence behind: a character the byte budget cannot hold is replaced
  // by a single space, the reference's "pad with ' ' if no room for
  // both bytes".
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 unsigned char *,
                 __cdecl,
                 _mbsnbset,
                 unsigned char *dst,
                 unsigned int val,
                 size_t count)
  {
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
