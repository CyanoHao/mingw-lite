#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // count *bytes* of src -- the secure twin of _mbsnbcpy, and the
  // converse of _mbsncpy_s above.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 errno_t,
                 __cdecl,
                 _mbsnbcpy_s,
                 unsigned char *dst,
                 size_t size,
                 const unsigned char *src,
                 size_t count)
  {
    return mbstring::ncopy_s(dst, size, src, count, true);
  }
} // namespace mingw_thunk
