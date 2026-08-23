#include <thunk/_common.h>
#include <thunk/u8crt/musl.h>

namespace mingw_thunk
{
  // Function form per ucrtbase.def: long *(__cdecl *)(void)
  __DEFINE_THUNK(api_ms_win_crt_time_l1_1_0, 0, long *, __cdecl, __dstbias)
  {
    return &musl::tz::get().dstbias;
  }
} // namespace mingw_thunk
