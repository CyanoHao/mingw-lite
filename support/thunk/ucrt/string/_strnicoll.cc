#include <thunk/_common.h>
#include <thunk/u8crt/c_utf8_tables.h>
#include <string.h>

namespace mingw_thunk
{
  // Case-insensitive count-limited collation == _strnicmp
  __DEFINE_THUNK(api_ms_win_crt_string_l1_1_0, 0, int, __cdecl, _strnicoll, const char *s1, const char *s2, size_t count)
  {
    return _strnicmp(s1, s2, count);
  }
} // namespace mingw_thunk
