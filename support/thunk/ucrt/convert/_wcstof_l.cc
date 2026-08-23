#include <thunk/_common.h>

#include <locale.h>
#include <stdlib.h>
#include <wchar.h>

namespace mingw_thunk
{
  // Locale argument ignored (api-set global principle 2).
  __DEFINE_THUNK(api_ms_win_crt_convert_l1_1_0,
                 0,
                 float,
                 __cdecl,
                 _wcstof_l,
                 const wchar_t *string,
                 wchar_t **end_ptr,
                 _locale_t locale)
  {
    (void)locale;
    return wcstof(string, end_ptr);
  }
} // namespace mingw_thunk
