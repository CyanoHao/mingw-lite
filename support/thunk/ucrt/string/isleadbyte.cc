#include <thunk/_common.h>
#include <thunk/u8crt/c_utf8_tables.h>

namespace mingw_thunk
{
  // UTF-8 is self-synchronizing — no lead bytes, always false
  __DEFINE_THUNK(api_ms_win_crt_string_l1_1_0, 0, int, __cdecl, isleadbyte, int c)
  {
    (void)c;
    return 0;
  }
} // namespace mingw_thunk
