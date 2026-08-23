#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // Append count *bytes* of src -- the secure twin of _mbsnbcat.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 errno_t,
                 __cdecl,
                 _mbsnbcat_s,
                 unsigned char *dst,
                 size_t size,
                 const unsigned char *src,
                 size_t count)
  {
    return mbstring::ncat_s(dst, size, src, count, true);
  }
} // namespace mingw_thunk
