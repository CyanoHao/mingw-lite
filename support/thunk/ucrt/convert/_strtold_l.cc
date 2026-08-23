#include <thunk/_common.h>

#include <locale.h>
#include <stdlib.h>

namespace mingw_thunk
{
  // Not declared by the mingw headers on i686 (F_LD64 gap) — the macro's
  // own extern declaration provides the face; locale argument ignored.
  __DEFINE_THUNK(api_ms_win_crt_convert_l1_1_0,
                 0,
                 long double,
                 __cdecl,
                 _strtold_l,
                 const char *string,
                 char **end_ptr,
                 _locale_t locale)
  {
    (void)locale;
    return strtold(string, end_ptr);
  }
} // namespace mingw_thunk
