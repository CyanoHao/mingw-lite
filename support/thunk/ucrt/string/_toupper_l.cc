#include <thunk/_common.h>
#include <thunk/u8crt/c_utf8_tables.h>
#include <locale.h>

namespace mingw_thunk
{
  // Locale ignored — same raw arithmetic fold
  __DEFINE_THUNK(api_ms_win_crt_string_l1_1_0, 0, int, __cdecl, _toupper_l, int c, _locale_t locale)
  {
    (void)locale;
    return c - ('a' - 'A');
  }
} // namespace mingw_thunk
