#include <thunk/_common.h>
#include <thunk/_no_thunk.h>

#include <string.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(api_ms_win_crt_string_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 strcoll,
                 const char *s1,
                 const char *s2)
  {
    return strcmp(s1, s2);
  }
} // namespace mingw_thunk
