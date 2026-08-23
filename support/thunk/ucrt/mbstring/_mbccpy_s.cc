#include <thunk/_common.h>

#include "mbs_char.h"

namespace mingw_thunk
{
  // The secure one-character copy.  Same protocol as the string
  // family's _s faces: EINVAL for a null or zero-size destination, a
  // null source after the destination is validated (the destination is
  // terminated and EINVAL reported), ERANGE when the destination is
  // too small for the character, and EILSEQ when the source is a lead
  // byte whose character never finishes -- in which case only the
  // terminator is written.  *pcopied, when the caller asks, is the
  // number of bytes written.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 errno_t,
                 __cdecl,
                 _mbccpy_s,
                 unsigned char *dst,
                 size_t size,
                 int *pcopied,
                 const unsigned char *src)
  {
    return mbstring::ccpy_s(dst, size, pcopied, src);
  }
} // namespace mingw_thunk
