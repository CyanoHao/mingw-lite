#include <thunk/_common.h>

#include "mbs_char.h"

namespace mingw_thunk
{
  // Katakana to hiragana by codepoint.  The reference subtracts 0xa1
  // from a Shift-JIS byte pair, which on a codepoint would answer
  // U+3001 for U+30A2 -- arithmetic on the wrong alphabet.  The two
  // kana blocks line up 0x60 apart, so the mapping is one subtraction,
  // and everything that is not in the katakana range (including the
  // long vowel mark U+30FC) comes back unchanged (D7c).
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 unsigned int,
                 __cdecl,
                 _mbctohira,
                 unsigned int c)
  {
    return mbstring::to_hira(c);
  }
} // namespace mingw_thunk
