#include <thunk/_common.h>

#include <windows.h>

namespace mingw_thunk
{
  // D9: the overlay's only MBCS state is UTF-8, self-reported as 65001
  // (native initial value is the ACP — 1252 in the wine probe — and
  // becomes whatever was last set; divergence noted).
  __DEFINE_THUNK(
      api_ms_win_crt_locale_l1_1_0, 0, int, __cdecl, _getmbcp, void)
  {
    return CP_UTF8;
  }
} // namespace mingw_thunk
