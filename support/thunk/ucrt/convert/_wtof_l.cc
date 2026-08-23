#include <thunk/_common.h>

#include <locale.h>
#include <stdlib.h>
#include <wchar.h>

namespace mingw_thunk
{
  // Locale argument ignored (api-set global principle 2).
  __DEFINE_THUNK(api_ms_win_crt_convert_l1_1_0,
                 0,
                 double,
                 __cdecl,
                 _wtof_l,
                 const wchar_t *string,
                 _locale_t locale)
  {
    (void)locale;
    return _wtof(string);
  }
} // namespace mingw_thunk
