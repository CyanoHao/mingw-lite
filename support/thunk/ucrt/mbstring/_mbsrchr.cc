#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // Last character equal to ch.  Unlike the reference, a truncated tail
  // is not reported as a hit when nothing else matched: the DBCS quirk
  // it reproduces there only fires on a string that cannot exist as
  // well-formed UTF-8.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 const unsigned char *,
                 __cdecl,
                 _mbsrchr,
                 const unsigned char *string,
                 unsigned int ch)
  {
    return mbstring::mbs_find(string, ch, true);
  }
} // namespace mingw_thunk
