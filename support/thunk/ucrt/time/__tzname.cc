#include <thunk/_common.h>
#include <thunk/u8crt/musl.h>

namespace mingw_thunk
{
  // Function form per ucrtbase.def (_o___tzname mangled signature:
  // char **(__cdecl *)(void)); the pointers are stable — the state
  // rewrites the buffers in place on _tzset (plan-2 M9)
  __DEFINE_THUNK(api_ms_win_crt_time_l1_1_0, 0, char **, __cdecl, __tzname)
  {
    return musl::tz::get().tzname;
  }
} // namespace mingw_thunk
