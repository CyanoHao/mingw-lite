#include <thunk/_common.h>
#include <thunk/u8crt/c_utf8_tables.h>
#include <locale.h>
#include <string.h>

namespace mingw_thunk
{
  // Locale ignored — identity transform
  __DEFINE_THUNK(api_ms_win_crt_string_l1_1_0, 0, size_t, __cdecl, _strxfrm_l, char *dst, const char *src, size_t count, _locale_t locale)
  {
    (void)locale;
    return strxfrm(dst, src, count);
  }
} // namespace mingw_thunk
