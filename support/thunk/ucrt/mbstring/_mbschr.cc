#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // First character equal to ch, which carries a whole scalar value --
  // the UTF-8 analogue of the DBCS 16-bit code the reference compares
  // against.  ch == 0 matches the terminator, which is how the reference
  // handles an embedded-NUL search.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 const unsigned char *,
                 __cdecl,
                 _mbschr,
                 const unsigned char *string,
                 unsigned int ch)
  {
    return mbstring::mbs_find(string, ch, false);
  }
} // namespace mingw_thunk
