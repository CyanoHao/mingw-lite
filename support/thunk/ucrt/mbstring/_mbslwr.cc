#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // A codepoint fold in place: ASCII through the byte table, everything
  // above through the invariant wide fold (plan-3 D7a), so U+3042 and
  // its uppercase partner meet.  A fold that would change a character's
  // encoded width cannot be done in place, so that character is left
  // alone and the face reports EILSEQ -- the reference's
  // LCMapStringA failure path, verbatim.
  // The reference spells the whole face as `_mbslwr_s_l(s, (size_t)-1)
  // == 0 ? s : nullptr`, which is reproduced here: the unbounded size
  // never trips the "not terminated" check, so only a real failure
  // answers null.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 unsigned char *,
                 __cdecl,
                 _mbslwr,
                 unsigned char *string)
  {
    return mbstring::case_s(string, (size_t)-1, 0) == 0 ? string : nullptr;
  }
} // namespace mingw_thunk
