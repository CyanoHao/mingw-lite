#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // count *characters* of src with no NUL padding: the secure form
  // truncates at min(count, length) and only checks that the result fits,
  // so a count of 0 resets the destination and reports success without
  // looking at src.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 errno_t,
                 __cdecl,
                 _mbsncpy_s,
                 unsigned char *dst,
                 size_t size,
                 const unsigned char *src,
                 size_t count)
  {
    return mbstring::ncopy_s(dst, size, src, count, false);
  }
} // namespace mingw_thunk
