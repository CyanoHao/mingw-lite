#include <thunk/_common.h>

#include <stdlib.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(msvcrt,
                 0,
                 void,
                 __cdecl,
                 searchenv,
                 const char *filename,
                 const char *envvar,
                 char *result)
  {
    _searchenv(filename, envvar, result);
  }
} // namespace mingw_thunk
