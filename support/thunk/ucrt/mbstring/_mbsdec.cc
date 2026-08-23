#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // The start of the character before the one ending just below current.
  // A null argument is EINVAL; current at or before string answers null
  // with errno untouched, the reference's `if (string >= current) return
  // nullptr`.  The walk runs forward rather than guessing backwards: a
  // backwards scan has to re-derive the alignment, and on a malformed
  // string the two answers can differ.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 const unsigned char *,
                 __cdecl,
                 _mbsdec,
                 const unsigned char *string,
                 const unsigned char *current)
  {
    return mbstring::mbs_dec(string, current);
  }
} // namespace mingw_thunk
