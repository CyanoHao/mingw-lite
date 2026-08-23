#include <thunk/_common.h>

#include "mbs_char.h"

namespace mingw_thunk
{
  // Hiragana to katakana, the mirror of _mbctohira: the same 0x60 in
  // the other direction, and non-kana input unchanged.  The two faces
  // are inverse on the kana blocks and identity on everything else, so
  // a round trip is exact on kana and a no-op off it.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 unsigned int,
                 __cdecl,
                 _mbctokata,
                 unsigned int c)
  {
    return mbstring::to_kata(c);
  }
} // namespace mingw_thunk
