#include <thunk/_common.h>

#include <locale.h>
#include <stdlib.h>

namespace mingw_thunk
{
  // Locale argument ignored — delegate to this layer's own wctomb
  // thunk.
  __DEFINE_THUNK(api_ms_win_crt_convert_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 _wctomb_l,
                 char *s,
                 wchar_t wc,
                 _locale_t locale)
  {
    (void)locale;
    return wctomb(s, wc);
  }
} // namespace mingw_thunk
