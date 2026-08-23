#include <thunk/_common.h>
#include <thunk/u8crt/c_utf8_tables.h>

namespace mingw_thunk
{
  // Checked fold (wine anchor: EOF passes through, >= 0x80
  // unchanged, non-letters unchanged)
  __DEFINE_THUNK(api_ms_win_crt_string_l1_1_0, 0, int, __cdecl, tolower, int c)
  {
    return i::u8_fold_lower(c);
  }
} // namespace mingw_thunk
