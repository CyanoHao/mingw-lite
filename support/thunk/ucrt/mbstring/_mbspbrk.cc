#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // The first character control *does* contain.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 const unsigned char *,
                 __cdecl,
                 _mbspbrk,
                 const unsigned char *string,
                 const unsigned char *control)
  {
    return mbstring::mbs_pbrk(string, control);
  }
} // namespace mingw_thunk
