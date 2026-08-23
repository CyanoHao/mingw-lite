#include <thunk/_common.h>
#include <thunk/u8crt/c_utf8_tables.h>

namespace mingw_thunk
{
  // ASCII byte fold compare; returns the folded byte difference at
  // the first mismatch (wine anchor: _stricmp("a","c") == -2);
  // bytes >= 0x80 compare raw (full Unicode folding is a P3
  // enhancement).  Covers the POSIX strcasecmp/strncasecmp header
  // aliases
  __DEFINE_THUNK(api_ms_win_crt_string_l1_1_0, 0, int, __cdecl, _stricmp, const char *s1, const char *s2)
  {
    const unsigned char *p1 = reinterpret_cast<const unsigned char *>(s1);
    const unsigned char *p2 = reinterpret_cast<const unsigned char *>(s2);
    int diff;
    while ((diff = i::u8_byte_lower(*p1) - i::u8_byte_lower(*p2)) == 0 &&
           *p1) {
      ++p1;
      ++p2;
    }
    return diff;
  }
} // namespace mingw_thunk
