#include <thunk/_common.h>
#include <thunk/u8crt/c_utf8_tables.h>

namespace mingw_thunk
{
  // Folded byte-block compare (wine anchor: _memicmp("AB","ac",2) == -1)
  __DEFINE_THUNK(api_ms_win_crt_string_l1_1_0, 0, int, __cdecl, _memicmp, const void *p1, const void *p2, size_t count)
  {
    const unsigned char *b1 = reinterpret_cast<const unsigned char *>(p1);
    const unsigned char *b2 = reinterpret_cast<const unsigned char *>(p2);
    for (size_t k = 0; k < count; k++) {
      int diff = i::u8_byte_lower(b1[k]) - i::u8_byte_lower(b2[k]);
      if (diff)
        return diff;
    }
    return 0;
  }
} // namespace mingw_thunk
