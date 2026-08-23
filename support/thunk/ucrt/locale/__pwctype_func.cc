#include <thunk/_common.h>
#include <thunk/u8crt/c_utf8_tables.h>

namespace mingw_thunk
{
  // M10: the wide table is NOT the narrow table (probe "PWCT": the high
  // half carries Latin-1 Unicode categories; 'A' has the composite
  // _ALPHA bit set).  u8_pwctype is the wine-anchored initial wide
  // table with the '\t' _BLANK correction.
  __DEFINE_THUNK(
      api_ms_win_crt_locale_l1_1_0, 0, unsigned short const *, __cdecl,
      __pwctype_func, void)
  {
    return &i::u8_pwctype[0];
  }
} // namespace mingw_thunk
