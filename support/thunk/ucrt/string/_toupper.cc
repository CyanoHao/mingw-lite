#include <thunk/_common.h>
#include <thunk/u8crt/c_utf8_tables.h>

namespace mingw_thunk
{
  // Raw arithmetic fold, no validation (wine anchor: 'a' -> 129,
  // '1' -> 81, 0xE9 -> 265, EOF -> 31; _toupper: 'A' -> 33,
  // EOF -> -33) — matches the ctype.h macro semantics
  __DEFINE_THUNK(api_ms_win_crt_string_l1_1_0, 0, int, __cdecl, _toupper, int c)
  {
    return c - ('a' - 'A');
  }
} // namespace mingw_thunk
