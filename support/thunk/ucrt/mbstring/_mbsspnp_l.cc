#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // Locale argument ignored; the _l twin of _mbsspnp.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 const unsigned char *,
                 __cdecl,
                 _mbsspnp_l,
                 const unsigned char *string,
                 const unsigned char *control,
                 _locale_t locale)
  {
    (void)locale;
    return mbstring::mbs_spnp(string, control);
  }
} // namespace mingw_thunk
