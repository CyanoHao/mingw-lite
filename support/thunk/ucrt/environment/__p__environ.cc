#include <thunk/_common.h>
#include <thunk/u8crt/musl.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(
      api_ms_win_crt_environment_l1_1_0, 0, char ***, __cdecl, __p__environ)
  {
    return &musl::__environ;
  }
} // namespace mingw_thunk
