#include <thunk/_common.h>

#include <errno.h>
#include <locale.h>
#include <stdlib.h>
#include <wchar.h>

namespace mingw_thunk
{
  // Locale argument ignored — delegate to this layer's own mbstowcs.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 size_t,
                 __cdecl,
                 _mbstowcs_l,
                 wchar_t *pwcs,
                 const char *s,
                 size_t n,
                 _locale_t locale)
  {
    (void)locale;
    return mbstowcs(pwcs, s, n);
  }
} // namespace mingw_thunk
