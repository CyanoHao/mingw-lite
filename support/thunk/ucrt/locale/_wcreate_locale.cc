#include <thunk/_common.h>
#include "_locale_sentinel.h"

#include <locale.h>

namespace mingw_thunk
{
  // D10 wide mirror of _create_locale.
  __DEFINE_THUNK(api_ms_win_crt_locale_l1_1_0,
                 0,
                 _locale_t,
                 __cdecl,
                 _wcreate_locale,
                 int category,
                 const wchar_t *locale)
  {
    if (!locale || category < 0 || category > 5)
      return nullptr;

    return &i::u8_locale_sentinel;
  }
} // namespace mingw_thunk
