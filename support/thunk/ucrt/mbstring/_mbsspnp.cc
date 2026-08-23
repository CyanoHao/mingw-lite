#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // A pointer to the first character control does not contain, or null
  // when the whole string is in the charset -- the reference's SBCS
  // shortcut answers exactly that.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 const unsigned char *,
                 __cdecl,
                 _mbsspnp,
                 const unsigned char *string,
                 const unsigned char *control)
  {
    return mbstring::mbs_spnp(string, control);
  }
} // namespace mingw_thunk
