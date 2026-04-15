#include "__p___argv.h"

#include <thunk/_common.h>
#include <thunk/string.h>

#include <stdlib.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(crtdll, 0, char ***, __cdecl, __p___argv)
  {
    return &i::u8_argv;
  }

  namespace i
  {
    char **u8_argv = nullptr;
  } // namespace i
} // namespace mingw_thunk
