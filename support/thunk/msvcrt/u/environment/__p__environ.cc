#include <thunk/_common.h>
#include <thunk/u8crt/musl.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(msvcrt, 0, char ***, __cdecl, __p__environ)
  {
    return &musl::__environ;
  }

  __DECLARE_FORCE_OVERRIDE_MINGW_EMU(__p__environ)
} // namespace mingw_thunk
