#include <thunk/_common.h>

#include <stdlib.h>

namespace mingw_thunk
{
  extern "C" errno_t __cdecl
  _searchenv_s(const char *file_name, const char *environment_variable,
               char *result_buffer, size_t result_count);

  // reference/ucrt/env/searchenv.cpp: the bounded engine with a
  // _MAX_PATH budget.
  __DEFINE_THUNK(api_ms_win_crt_environment_l1_1_0,
                 0,
                 void,
                 __cdecl,
                 _searchenv,
                 const char *file_name,
                 const char *environment_variable,
                 char *result_buffer)
  {
    _searchenv_s(file_name, environment_variable, result_buffer, _MAX_PATH);
  }
} // namespace mingw_thunk
