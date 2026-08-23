#include <thunk/_common.h>
#include <thunk/u8crt/musl.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(msvcrt, 0, char *, __cdecl, getenv, const char *name)
  {
    return musl::getenv(name);
  }
} // namespace mingw_thunk
