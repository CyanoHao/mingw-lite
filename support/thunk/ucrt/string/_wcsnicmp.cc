#include <thunk/_common.h>
#include <thunk/u8crt/c_utf8_tables.h>
#include <wchar.h>

namespace mingw_thunk
{
  // Count-limited _wcsicmp (wine anchor shape: Ab vs aC -> -1)
  __DEFINE_THUNK(api_ms_win_crt_string_l1_1_0, 0, int, __cdecl, _wcsnicmp, const wchar_t *s1, const wchar_t *s2, size_t count)
  {
    int diff = 0;
    while (count &&
           (diff = static_cast<int>(i::u8_wfold_lower(*s1)) -
                   static_cast<int>(i::u8_wfold_lower(*s2))) == 0 &&
           *s1) {
      ++s1;
      ++s2;
      --count;
    }
    return diff;
  }
} // namespace mingw_thunk
