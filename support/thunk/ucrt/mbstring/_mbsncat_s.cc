#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // Append count *characters* of src, no terminator padding.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 errno_t,
                 __cdecl,
                 _mbsncat_s,
                 unsigned char *dst,
                 size_t size,
                 const unsigned char *src,
                 size_t count)
  {
    return mbstring::ncat_s(dst, size, src, count, false);
  }
} // namespace mingw_thunk
