#include <thunk/_common.h>
#include <thunk/u8crt/c_utf8_tables.h>
#include <string.h>

namespace mingw_thunk
{
  // Case-insensitive code point order == the _stricmp fold compare
  // (wine anchor: _stricoll("A","b") == -1)
  __DEFINE_THUNK(api_ms_win_crt_string_l1_1_0, 0, int, __cdecl, _stricoll, const char *s1, const char *s2)
  {
    return _stricmp(s1, s2);
  }
} // namespace mingw_thunk
