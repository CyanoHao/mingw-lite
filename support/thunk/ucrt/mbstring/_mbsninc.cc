#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // Advance count characters.  The reference reaches the same answer by
  // delegating to _mbsnbcnt, so count counts characters, not bytes.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 const unsigned char *,
                 __cdecl,
                 _mbsninc,
                 const unsigned char *string,
                 size_t count)
  {
    if (!string)
      return nullptr;
    return string + mbstring::mbs_nbcnt(string, count);
  }
} // namespace mingw_thunk
