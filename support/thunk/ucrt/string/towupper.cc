#include <thunk/_common.h>

#include <wctype.h>
#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(
      api_ms_win_crt_string_l1_1_0, 0, wint_t, __cdecl, towupper, wint_t wc)
  {
    wchar_t src = wc;
    wchar_t dst = src;
    if (LCMapStringW(LOCALE_INVARIANT, LCMAP_UPPERCASE, &src, 1, &dst, 1) == 1)
      return dst;
    return wc;
  }
} // namespace mingw_thunk
