#include <thunk/_common.h>
#include <thunk/u8crt/c_utf8_tables.h>
#include <wchar.h>

namespace mingw_thunk
{
  // Unicode-aware fold compare (C.UTF-8 divergence by design: the
  // C-locale native folds ASCII only — wine anchor: _wcsicmp(C9,E9)
  // == -32 on native, 0 here).  Returns the folded difference (wine
  // anchor shape: 'b' vs 'd' -> -2)
  __DEFINE_THUNK(api_ms_win_crt_string_l1_1_0, 0, int, __cdecl, _wcsicmp, const wchar_t *s1, const wchar_t *s2)
  {
    for (;;) {
      int diff = static_cast<int>(i::u8_wfold_lower(*s1)) -
                 static_cast<int>(i::u8_wfold_lower(*s2));
      if (diff)
        return diff;
      if (!*s1)
        return 0;
      ++s1;
      ++s2;
    }
  }
} // namespace mingw_thunk
