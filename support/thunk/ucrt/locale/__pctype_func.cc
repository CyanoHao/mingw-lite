#include <thunk/_common.h>
#include <thunk/u8crt/c_utf8_tables.h>

namespace mingw_thunk
{
  // M8 table promotion closure: __pctype_func now serves the static
  // C.UTF-8 narrow table directly (native walks an ACP-dependent table;
  // ours is fixed — bits >= 0x80 are all zero in the C state).
  __DEFINE_THUNK(
      api_ms_win_crt_locale_l1_1_0, 0, unsigned short const *, __cdecl,
      __pctype_func, void)
  {
    return &i::u8_pctype[0];
  }
} // namespace mingw_thunk
