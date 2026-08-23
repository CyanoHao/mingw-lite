#include <thunk/_common.h>
#include <thunk/u8crt/c_utf8_tables.h>

namespace mingw_thunk
{
  // Count-limited _stricmp (wine anchor: _strnicmp("Ab","aC",2) == -1)
  __DEFINE_THUNK(api_ms_win_crt_string_l1_1_0, 0, int, __cdecl, _strnicmp, const char *s1, const char *s2, size_t count)
  {
    const unsigned char *p1 = reinterpret_cast<const unsigned char *>(s1);
    const unsigned char *p2 = reinterpret_cast<const unsigned char *>(s2);
    int diff = 0;
    while (count && (diff = i::u8_byte_lower(*p1) - i::u8_byte_lower(*p2)) == 0 &&
           *p1) {
      ++p1;
      ++p2;
      --count;
    }
    return diff;
  }
} // namespace mingw_thunk
