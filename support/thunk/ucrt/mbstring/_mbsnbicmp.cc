#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // Folded code-point order over at most count bytes.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 _mbsnbicmp,
                 const unsigned char *s1,
                 const unsigned char *s2,
                 size_t count)
  {
    return mbstring::mbs_compare(
        s1, s2, count, mbstring::CMP_FOLD, mbstring::BND_BYTES);
  }
} // namespace mingw_thunk
