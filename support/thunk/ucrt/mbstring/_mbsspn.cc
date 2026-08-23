#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // Byte offset of the first character control does not contain.  The
  // charset is walked a whole character at a time, so a delimiter is a
  // delimiter in the UTF-8 sense and not a byte that happens to appear
  // inside one.  An incomplete entry at the end of the charset is
  // skipped rather than treated as a match-everything, the way the
  // reference's "lead byte followed by NUL" branch is: that branch is
  // an artifact of a dud DBCS pair, not a rule.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 size_t,
                 __cdecl,
                 _mbsspn,
                 const unsigned char *string,
                 const unsigned char *control)
  {
    return mbstring::mbs_spn(string, control);
  }
} // namespace mingw_thunk
