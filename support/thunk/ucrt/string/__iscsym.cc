#include <thunk/_common.h>
#include <thunk/u8crt/c_utf8_tables.h>

namespace mingw_thunk
{
  // alnum or underscore (wine anchors: '_'/'1' -> 1, matches)
  __DEFINE_THUNK(api_ms_win_crt_string_l1_1_0, 0, int, __cdecl, __iscsym, int c)
  {
    return i::u8_is(c, i::M_UPPER | i::M_LOWER | i::M_DIGIT) ||
           c == '_';
  }
} // namespace mingw_thunk
