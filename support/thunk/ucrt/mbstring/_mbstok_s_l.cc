#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // Locale argument ignored; the _l twin of _mbstok_s.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 unsigned char *,
                 __cdecl,
                 _mbstok_s_l,
                 unsigned char *string,
                 const unsigned char *control,
                 unsigned char **context,
                 _locale_t locale)
  {
    (void)locale;
    return mbstring::tok_split(string, control, context);
  }
} // namespace mingw_thunk
