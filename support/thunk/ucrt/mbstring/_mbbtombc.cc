#include <thunk/_common.h>

#include "mbs_char.h"

namespace mingw_thunk
{
  // Shift-JIS single-to-double-byte conversion.  The reference guards
  // on the kanji codepage and returns the input for every other one,
  // so under UTF-8 its 250-entry table is never reached and the answer
  // is the input.  Kept as identity rather than refused: emitting
  // Shift-JIS bytes from a UTF-8 layer would be encoding corruption,
  // which is the deliberate half of D7b.  errno untouched.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 unsigned int,
                 __cdecl,
                 _mbbtombc,
                 unsigned int c)
  {
    return mbstring::bbtombc(c);
  }
} // namespace mingw_thunk
