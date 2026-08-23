#include <thunk/_common.h>
#include <thunk/u8crt/c_utf8_tables.h>
#include <locale.h>

namespace mingw_thunk
{
  // Locale argument ignored — delegate to the fixed C.UTF-8 table
  __DEFINE_THUNK(api_ms_win_crt_string_l1_1_0, 0, int, __cdecl, _isprint_l, int c, _locale_t locale)
  {
    (void)locale;
    return i::u8_is(c, i::M_UPPER | i::M_LOWER | i::M_DIGIT | i::M_PUNCT | i::M_SPACE);
  }
} // namespace mingw_thunk
