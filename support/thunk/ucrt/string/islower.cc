#include <thunk/_common.h>
#include <thunk/u8crt/c_utf8_tables.h>

namespace mingw_thunk
{
  // C.UTF-8 static table (M8): ASCII decides, >= 0x80 never
  // matches, out-of-range (EOF included) is 0 — native state can
  // drift per ACP/locale, ours is fixed
  __DEFINE_THUNK(api_ms_win_crt_string_l1_1_0, 0, int, __cdecl, islower, int c)
  {
    return i::u8_is(c, i::M_LOWER);
  }
} // namespace mingw_thunk
