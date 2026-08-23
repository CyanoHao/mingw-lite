#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // Whole characters inside the first byte_count bytes.  A character the
  // bound would split is not counted and ends the walk, the reference's
  // `!bcnt--` break; a truncated tail likewise contributes nothing.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 size_t,
                 __cdecl,
                 _mbsnccnt,
                 const unsigned char *string,
                 size_t byte_count)
  {
    return mbstring::mbs_nccnt(string, byte_count);
  }
} // namespace mingw_thunk
