#include <thunk/_common.h>
#include <thunk/u8crt/musl.h>

namespace mingw_thunk
{
  // Function form per ucrtbase.def: int *(__cdecl *)(void)
  __DEFINE_THUNK(api_ms_win_crt_time_l1_1_0, 0, int *, __cdecl, __daylight)
  {
    return &musl::tz::get().daylight;
  }
} // namespace mingw_thunk
