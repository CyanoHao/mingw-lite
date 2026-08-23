#include <thunk/_common.h>
#include <thunk/u8crt/c_utf8_tables.h>
#include <locale.h>

namespace mingw_thunk
{
  // Locale ignored — fixed table probe
  __DEFINE_THUNK(api_ms_win_crt_string_l1_1_0, 0, int, __cdecl, _isctype_l, int c, int mask, _locale_t locale)
  {
    (void)locale;
    return i::u8_is(c, static_cast<unsigned>(mask));
  }
} // namespace mingw_thunk
