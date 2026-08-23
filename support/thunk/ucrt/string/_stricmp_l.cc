#include <thunk/_common.h>
#include <thunk/u8crt/c_utf8_tables.h>
#include <locale.h>
#include <string.h>

namespace mingw_thunk
{
  // Locale argument ignored — delegate to this layer's own thunk
  __DEFINE_THUNK(api_ms_win_crt_string_l1_1_0, 0, int, __cdecl, _stricmp_l, const char *s1, const char *s2, _locale_t locale)
  {
    (void)locale;
    return _stricmp(s1, s2);
  }
} // namespace mingw_thunk
