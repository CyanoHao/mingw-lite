#include <thunk/_common.h>
#include <thunk/string.h>
#include <thunk/utf8-musl.h>

#include <stdlib.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(
      api_ms_win_crt_environment_l1_1_0, 0, char ***, __cdecl, __p___argv)
  {
    return &musl::utf8_argv;
  }

  __DECLARE_FORCE_OVERRIDE_MINGW_EMU(__p___argv)
} // namespace mingw_thunk
