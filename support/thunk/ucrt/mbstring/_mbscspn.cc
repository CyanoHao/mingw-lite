#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // The complement of _mbsspn, as _mbspbrk is the complement of
  // _mbsspnp: the byte offset of the first character control *does*
  // contain.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 size_t,
                 __cdecl,
                 _mbscspn,
                 const unsigned char *string,
                 const unsigned char *control)
  {
    return mbstring::mbs_cspn(string, control);
  }
} // namespace mingw_thunk
