#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // The case-insensitive collation twin of _mbscoll, mirroring M8's
  // _stricoll -> _stricmp decision.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 _mbsicoll,
                 const unsigned char *s1,
                 const unsigned char *s2)
  {
    return mbstring::mbs_compare(
        s1, s2, 0, mbstring::CMP_FOLD, mbstring::BND_STR);
  }
} // namespace mingw_thunk
