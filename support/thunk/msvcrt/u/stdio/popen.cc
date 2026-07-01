#include <thunk/_common.h>
#include <thunk/string.h>

#include <errno.h>
#include <stdio.h>

#undef popen

namespace mingw_thunk
{
  __DEFINE_THUNK(
      msvcrt, 0, FILE *, __cdecl, popen, const char *command, const char *type)
  {
    return _popen(command, type);
  }
} // namespace mingw_thunk
