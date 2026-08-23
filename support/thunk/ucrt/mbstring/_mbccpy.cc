#include <thunk/_common.h>

#include "mbs_char.h"

namespace mingw_thunk
{
  // Copy one whole character to the destination.  The reference spells
  // this as its secure body called with a size of 2 -- the widest
  // character the DBCS codepage has -- so the destination owes one
  // character's worth of room, which under UTF-8 is four bytes.  The
  // destination gets the character alone and no terminator, exactly as
  // the reference's two stores do.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 void,
                 __cdecl,
                 _mbccpy,
                 unsigned char *dst,
                 const unsigned char *src)
  {
    mbstring::ccpy(dst, src);
  }
} // namespace mingw_thunk
