#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // The scalar at current.  The reference returns a two-byte DBCS code;
  // the UTF-8 analogue carries the whole scalar value, which is what the
  // unsigned int return type is wide enough for.  A malformed or
  // truncated character answers 0, and a null argument is EINVAL.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 unsigned int,
                 __cdecl,
                 _mbsnextc,
                 const unsigned char *current)
  {
    return mbstring::mbs_nextc(current);
  }
} // namespace mingw_thunk
