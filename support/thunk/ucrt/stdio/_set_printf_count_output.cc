#include <thunk/_common.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(api_ms_win_crt_stdio_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 _set_printf_count_output,
                 int enable [[maybe_unused]])
  {
    return 1;
  }
} // namespace mingw_thunk
