#include <thunk/_common.h>
#include "_locale_sentinel.h"

#include <locale.h>

namespace mingw_thunk
{
  // D10: lenient sentinel — native returns NULL only for null name /
  // category outside [0,5] (probe B) and for enumerable bad names
  // ("zzz_ZZ" -> NULL); the overlay cannot enumerate the name list, so
  // every accepted call returns the sentinel (divergence for bogus
  // names and "" noted in u8crt-api-set §3.4).  Native allocates a
  // fresh object per call; ours returns the same pointer forever.
  __DEFINE_THUNK(api_ms_win_crt_locale_l1_1_0,
                 0,
                 _locale_t,
                 __cdecl,
                 _create_locale,
                 int category,
                 const char *locale)
  {
    if (!locale || category < 0 || category > 5)
      return nullptr;

    return &i::u8_locale_sentinel;
  }
} // namespace mingw_thunk
