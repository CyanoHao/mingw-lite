#include <thunk/_common.h>

#include <locale.h>
#include <stdlib.h>

namespace mingw_thunk
{
  // Locale argument ignored (api-set global principle 2).
  __DEFINE_THUNK(api_ms_win_crt_convert_l1_1_0,
                 0,
                 double,
                 __cdecl,
                 _strtod_l,
                 const char *string,
                 char **end_ptr,
                 _locale_t locale)
  {
    (void)locale;
    return strtod(string, end_ptr);
  }
} // namespace mingw_thunk
