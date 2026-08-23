#include <thunk/_common.h>

#include <locale.h>
#include <stdlib.h>
#include <wchar.h>

namespace mingw_thunk
{
  // Not declared by the mingw headers on i686 (F_LD64 gap); locale
  // argument ignored.
  __DEFINE_THUNK(api_ms_win_crt_convert_l1_1_0,
                 0,
                 long double,
                 __cdecl,
                 _wcstold_l,
                 const wchar_t *string,
                 wchar_t **end_ptr,
                 _locale_t locale)
  {
    (void)locale;
    return wcstold(string, end_ptr);
  }
} // namespace mingw_thunk
