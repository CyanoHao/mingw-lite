#include <thunk/_common.h>

#include <locale.h>
#include <stdlib.h>

namespace mingw_thunk
{
  extern "C" errno_t __cdecl
  wctomb_s(int *size_converted, char *s, size_t size, wchar_t wc);

  // Locale argument ignored — delegate to this layer's own wctomb_s
  // thunk.
  __DEFINE_THUNK(api_ms_win_crt_convert_l1_1_0,
                 0,
                 errno_t,
                 __cdecl,
                 _wctomb_s_l,
                 int *size_converted,
                 char *s,
                 size_t size,
                 wchar_t wc,
                 _locale_t locale)
  {
    (void)locale;
    return wctomb_s(size_converted, s, size, wc);
  }
} // namespace mingw_thunk
