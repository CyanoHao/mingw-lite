#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // malloc and copy.  A null string answers null with errno untouched
  // (wine anchor) and an empty string still allocates, so the caller can
  // free unconditionally.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 unsigned char *,
                 __cdecl,
                 _mbsdup,
                 const unsigned char *string)
  {
    return mbstring::mbs_dup(string);
  }
} // namespace mingw_thunk
