#include <thunk/_common.h>
#include <thunk/u8crt/c_utf8_tables.h>

namespace mingw_thunk
{
  // alpha or underscore (wine anchors: '1' -> 0, 'A' -> 1)
  __DEFINE_THUNK(api_ms_win_crt_string_l1_1_0, 0, int, __cdecl, __iscsymf, int c)
  {
    return i::u8_is(c, i::M_UPPER | i::M_LOWER) || c == '_';
  }
} // namespace mingw_thunk
