#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // count *characters* of val in place, with the secure-crt protocol:
  // EINVAL for a null or zero-size pair, EINVAL + reset when the string
  // has no terminator inside size, EILSEQ when a value cannot be laid
  // down, and a zeroed tail on success.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 errno_t,
                 __cdecl,
                 _mbsnset_s,
                 unsigned char *dst,
                 size_t size,
                 unsigned int val,
                 size_t count)
  {
    return mbstring::set_s(dst, size, val, count, false);
  }
} // namespace mingw_thunk
