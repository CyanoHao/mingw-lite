#include <thunk/_common.h>

#include "mbs_char.h"

namespace mingw_thunk
{
  // JIS to JMS (Shift-JIS) conversion.  Same kanji-codepage guard as
  // the two above, so outside cp932 the input is the whole answer --
  // including the reference's EILSEQ for a code outside its printable
  // range, which its early return never reaches.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 unsigned int,
                 __cdecl,
                 _mbcjistojms,
                 unsigned int c)
  {
    return mbstring::jistojms(c);
  }
} // namespace mingw_thunk
