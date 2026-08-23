#include <thunk/_common.h>
#include <thunk/u8crt/c_utf8_tables.h>
#include <string.h>

namespace mingw_thunk
{
  // Case-sensitive code point order == strncmp (native strncmp is a
  // state-free R2 plain; wine anchor: equal-prefix case -> 0)
  __DEFINE_THUNK(api_ms_win_crt_string_l1_1_0, 0, int, __cdecl, _strncoll, const char *s1, const char *s2, size_t count)
  {
    return strncmp(s1, s2, count);
  }
} // namespace mingw_thunk
