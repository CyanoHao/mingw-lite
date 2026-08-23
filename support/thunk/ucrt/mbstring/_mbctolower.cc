#include <thunk/_common.h>

#include "mbs_char.h"

namespace mingw_thunk
{
  // Fold one character to lower case.  The reference packs a DBCS code
  // into the argument and unpacks it to fold two bytes; a UTF-8
  // sequence does not fit that slot, so the whole scalar value travels
  // in the argument and nothing is packed (plan-3 D7a).  ASCII folds
  // through the byte table, everything above through the codepoint
  // folder -- which is what makes U+00C0 answer U+00E0 while a kana
  // answers itself, since the invariant fold does not move kana.
  // A value that is not a scalar (> U+10FFFF, or a surrogate) answers
  // itself with errno untouched.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 unsigned int,
                 __cdecl,
                 _mbctolower,
                 unsigned int c)
  {
    return mbstring::char_lower(c);
  }
} // namespace mingw_thunk
