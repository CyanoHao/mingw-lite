#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // The unbounded window, so this is the character count the reference's
  // common_mbstrlen_l returns -- and (size_t)-1 with EILSEQ when any
  // character is malformed, which the old _mbslen above never reports.
  // wine anchor: the SBCS shortcut's byte count, which is the same
  // number under a single-byte codepage.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 size_t,
                 __cdecl,
                 _mbstrlen,
                 const char *string)
  {
    return mbstring::mbs_strlen(string, (size_t)-1);
  }
} // namespace mingw_thunk
