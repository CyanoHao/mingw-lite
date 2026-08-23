#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // Reverse by whole characters, so CJK text comes back in reverse
  // character order where _strrev would have reversed bytes and produced
  // mojibake.  A truncated tail is the reference's unsolvable case --
  // reversing it would attach the lead byte to the character before it --
  // so the string is truncated there and EINVAL is reported, which is
  // exactly the repair the reference performs.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 unsigned char *,
                 __cdecl,
                 _mbsrev,
                 unsigned char *string)
  {
    return mbstring::mbs_rev(string);
  }
} // namespace mingw_thunk
