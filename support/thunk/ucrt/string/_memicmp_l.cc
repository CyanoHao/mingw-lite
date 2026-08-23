#include <thunk/_common.h>
#include <thunk/u8crt/c_utf8_tables.h>
#include <locale.h>
#include <string.h>

namespace mingw_thunk
{
  // Locale argument ignored — delegate to this layer's own thunk
  __DEFINE_THUNK(api_ms_win_crt_string_l1_1_0, 0, int, __cdecl, _memicmp_l, const void *p1, const void *p2, size_t count, _locale_t locale)
  {
    (void)locale;
    return _memicmp(p1, p2, count);
  }
} // namespace mingw_thunk
