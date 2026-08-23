#include <thunk/_common.h>

#include "wcs_coll.h"

namespace mingw_thunk
{
  // Locale argument ignored; the _l twin of wcscoll.  The reference
  // reaches its collation through the locale, and this layer has one
  // collation, so there is nothing for the argument to select.
  __DEFINE_THUNK(api_ms_win_crt_string_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 _wcscoll_l,
                 const wchar_t *s1,
                 const wchar_t *s2,
                 _locale_t locale)
  {
    (void)locale;
    if (!s1 || !s2) {
      _set_errno(EINVAL);
      return wcoll::kNlsCmpError;
    }
    return wcoll::compare(s1, s2, (size_t)-1, false, true);
  }
} // namespace mingw_thunk
