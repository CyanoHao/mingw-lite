#include <thunk/_common.h>

#include <stdlib.h>

namespace mingw_thunk
{
  // Non-secure variant: delegates to _searchenv_s assuming a _MAX_PATH result
  // buffer, mirroring upstream UCRT (env/searchenv.cpp).
  __DEFINE_THUNK(msvcrt,
                 0,
                 void,
                 __cdecl,
                 _searchenv,
                 const char *filename,
                 const char *envvar,
                 char *result)
  {
    _searchenv_s(filename, envvar, result, _MAX_PATH);
  }
} // namespace mingw_thunk
