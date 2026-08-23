#include <thunk/_common.h>

#include "mbs_char.h"

namespace mingw_thunk
{
  // JMS (Shift-JIS) to JIS conversion, the reverse of _mbcjistojms and
  // identity outside the kanji codepage.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 unsigned int,
                 __cdecl,
                 _mbcjmstojis,
                 unsigned int c)
  {
    return mbstring::jmstojis(c);
  }
} // namespace mingw_thunk
