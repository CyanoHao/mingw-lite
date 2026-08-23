#include <thunk/_common.h>
#include <thunk/u8crt/musl.h>

namespace mingw_thunk
{
  // Function form per ucrtbase.def: long *(__cdecl *)(void);
  // seconds west of UTC (XXX-8 -> -28800, EST5 -> +18000)
  __DEFINE_THUNK(api_ms_win_crt_time_l1_1_0, 0, long *, __cdecl, __timezone)
  {
    return &musl::tz::get().timezone;
  }
} // namespace mingw_thunk
