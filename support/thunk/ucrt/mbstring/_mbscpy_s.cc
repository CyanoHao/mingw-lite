#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // The whole source, or ERANGE.  A null source resets the buffer and
  // is EINVAL; a truncated final character clears the copied prefix and
  // is EILSEQ, the reference's `_ISMBBLEADPREFIX` repair generalised to
  // every UTF-8 width (D9w).  A capacity check that runs *before* any
  // byte is written is why the ERANGE path cannot overrun the caller's
  // buffer.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 errno_t,
                 __cdecl,
                 _mbscpy_s,
                 unsigned char *dst,
                 size_t size,
                 const unsigned char *src)
  {
    return mbstring::cpy_s(dst, size, src);
  }
} // namespace mingw_thunk
