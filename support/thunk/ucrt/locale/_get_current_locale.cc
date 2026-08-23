#include <thunk/_common.h>
#include "_locale_sentinel.h"

#include <locale.h>

namespace mingw_thunk
{
  namespace i
  {
    // The single process-wide sentinel (see _locale_sentinel.h).
    _locale_tstruct u8_locale_sentinel = {};
  } // namespace i

  // D5/D10: the one and only locale object.  Native hands out a fresh
  // allocation per call; the sentinel identity (== _create_locale's
  // return) is the documented overlay contract.
  __DEFINE_THUNK(api_ms_win_crt_locale_l1_1_0,
                 0,
                 _locale_t,
                 __cdecl,
                 _get_current_locale,
                 void)
  {
    return &i::u8_locale_sentinel;
  }
} // namespace mingw_thunk
