#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // Append the whole source.  A destination with no terminator inside
  // size is EINVAL with the buffer reset, which is the reference's
  // comment about not being able to look past the buffer to tell
  // whether it ended in a dud character.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 errno_t,
                 __cdecl,
                 _mbscat_s,
                 unsigned char *dst,
                 size_t size,
                 const unsigned char *src)
  {
    return mbstring::cat_s(dst, size, src);
  }
} // namespace mingw_thunk
