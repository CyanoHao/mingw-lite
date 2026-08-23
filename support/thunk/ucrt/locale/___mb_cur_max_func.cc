#include <thunk/_common.h>

#include <stdlib.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(
      api_ms_win_crt_locale_l1_1_0, 0, int, __cdecl, ___mb_cur_max_func, void)
  {
    return 4;
  }
} // namespace mingw_thunk
