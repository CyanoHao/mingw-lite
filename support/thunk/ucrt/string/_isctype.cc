#include <thunk/_common.h>
#include <thunk/u8crt/c_utf8_tables.h>

namespace mingw_thunk
{
  // Fixed C.UTF-8 table probe (wine anchors: in-range -> masked bits,
  // out-of-range incl. -1/0x100/0x4F60 -> 0); native consults the
  // drift-prone locale pctype
  __DEFINE_THUNK(api_ms_win_crt_string_l1_1_0, 0, int, __cdecl, _isctype, int c, int mask)
  {
    return i::u8_is(c, static_cast<unsigned>(mask));
  }
} // namespace mingw_thunk
