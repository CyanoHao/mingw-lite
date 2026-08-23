#include <thunk/_common.h>

#include <locale.h>
#include <stdlib.h>

namespace mingw_thunk
{
  // Locale argument ignored (api-set global principle 2).
  __DEFINE_THUNK(api_ms_win_crt_convert_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 _atoldbl_l,
                 _LDOUBLE *result,
                 char *string,
                 _locale_t locale)
  {
    (void)locale;
    return _atoldbl(result, string);
  }
} // namespace mingw_thunk
