#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // One character forward.  The reference's flat "+2 for a lead byte" is
  // a DBCS pair assumption; under UTF-8 the lead byte's declared width
  // is the answer, so a three-byte character advances three (plan-3 5.1).
  // A character the terminator cuts short steps to the terminator, which
  // is the reference's "the second byte was the end of the string, so do
  // not take the second step" read at UTF-8 width; anything else that
  // starts no character advances one byte, which is the wine anchor's
  // SBCS shortcut and keeps the face total (wine crashes on a null).
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 const unsigned char *,
                 __cdecl,
                 _mbsinc,
                 const unsigned char *current)
  {
    return mbstring::mbs_inc(current);
  }
} // namespace mingw_thunk
