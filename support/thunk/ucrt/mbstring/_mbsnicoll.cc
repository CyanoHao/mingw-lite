#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // The collation twin of _mbsnicmp.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 _mbsnicoll,
                 const unsigned char *s1,
                 const unsigned char *s2,
                 size_t count)
  {
    return mbstring::mbs_compare(
        s1, s2, count, mbstring::CMP_FOLD, mbstring::BND_CHARS);
  }
} // namespace mingw_thunk
