#include <thunk/_common.h>

#include "mbs_char.h"

namespace mingw_thunk
{
  // Fold one character to upper case, the same shape as _mbctolower.
  // The pair is deliberately not a mirrored table: the folds are
  // looked up per direction, so a character whose upper case folds
  // back to something else is still reported the way the table says.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 unsigned int,
                 __cdecl,
                 _mbctoupper,
                 unsigned int c)
  {
    return mbstring::char_upper(c);
  }
} // namespace mingw_thunk
