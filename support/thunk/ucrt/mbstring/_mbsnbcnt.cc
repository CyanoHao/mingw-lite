#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // Bytes occupied by the first char_count characters.  Never splits a
  // character and never counts a truncated tail: the reference's
  // `--p; break` repair leaves the dud lead byte out of the total.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 size_t,
                 __cdecl,
                 _mbsnbcnt,
                 const unsigned char *string,
                 size_t char_count)
  {
    return mbstring::mbs_nbcnt(string, char_count);
  }
} // namespace mingw_thunk
