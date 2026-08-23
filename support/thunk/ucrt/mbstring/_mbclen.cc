#include <thunk/_common.h>

#include "mbs_char.h"

namespace mingw_thunk
{
  // The byte length of the character at the pointer.  The reference
  // answers 2 only for a lead byte with a byte behind it, which is a
  // DBCS pair; here the whole well-formed sequence counts, so two,
  // three and four byte characters answer their own width (D8d).
  // A truncated sequence, a stray trail byte or an overlong form
  // answers 1 -- the reference's conservative guard, kept because a
  // half character must not be reported as a whole one.  A null
  // pointer answers 0 and EINVAL rather than faulting.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 size_t,
                 __cdecl,
                 _mbclen,
                 const unsigned char *string)
  {
    return mbstring::clen(string);
  }
} // namespace mingw_thunk
