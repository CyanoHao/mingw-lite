#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // Code-point order, whole string.  The reference normalises to
  // -1/0/1 instead of returning a raw difference and so does this.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 _mbscmp,
                 const unsigned char *s1,
                 const unsigned char *s2)
  {
    return mbstring::mbs_compare(
        s1, s2, 0, mbstring::CMP_RAW, mbstring::BND_STR);
  }
} // namespace mingw_thunk
