#include <thunk/_common.h>

#include <locale.h>

namespace mingw_thunk
{
  // D10: freeing the sentinel is a no-op (null tolerated, probe B).
  __DEFINE_THUNK(api_ms_win_crt_locale_l1_1_0,
                 0,
                 void,
                 __cdecl,
                 _free_locale,
                 _locale_t locale)
  {
    (void)locale;
  }
} // namespace mingw_thunk
