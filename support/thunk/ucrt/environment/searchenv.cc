#include <thunk/_common.h>

#include <stdlib.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(api_ms_win_crt_environment_l1_1_0,
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
