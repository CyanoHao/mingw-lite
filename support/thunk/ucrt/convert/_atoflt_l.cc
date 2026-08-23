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
                 _atoflt_l,
                 _CRT_FLOAT *result,
                 char *string,
                 _locale_t locale)
  {
    (void)locale;
    return _atoflt(result, string);
  }
} // namespace mingw_thunk
