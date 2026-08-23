#include <thunk/_common.h>
#include <thunk/u8crt/musl.h>

namespace mingw_thunk
{
  // One truth source: re-read TZ (POSIX subset) / the registry tz
  // (plan-2 M9).  Native _tzset walks the localized display-name
  // machinery; ours never leaves the C.UTF-8 state.
  __DEFINE_THUNK(api_ms_win_crt_time_l1_1_0, 0, void, __cdecl, _tzset)
  {
    musl::tz::reinit();
  }
} // namespace mingw_thunk
