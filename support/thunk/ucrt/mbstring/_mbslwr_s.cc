#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // The secure form, wine-anchored end to end: lwr_s("AbC",4) == 0,
  // lwr_s("AbC",3) == EINVAL with the buffer reset, lwr_s("AbC",0) ==
  // EINVAL with the buffer *untouched*, lwr_s(NULL,0) == 0,
  // lwr_s(NULL,4) == EINVAL, and an over-large size is 0, because the
  // string is still terminated inside it.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 errno_t,
                 __cdecl,
                 _mbslwr_s,
                 unsigned char *string,
                 size_t size)
  {
    return mbstring::case_s(string, size, 0);
  }
} // namespace mingw_thunk
