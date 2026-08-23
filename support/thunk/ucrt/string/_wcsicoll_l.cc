#include <thunk/_common.h>

#include "wcs_coll.h"

namespace mingw_thunk
{
  // Locale argument ignored; the _l twin of _wcsicoll.
  __DEFINE_THUNK(api_ms_win_crt_string_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 _wcsicoll_l,
                 const wchar_t *s1,
                 const wchar_t *s2,
                 _locale_t locale)
  {
    (void)locale;
    if (!s1 || !s2) {
      _set_errno(EINVAL);
      return wcoll::kNlsCmpError;
    }
    return wcoll::compare(s1, s2, (size_t)-1, true, false);
  }
} // namespace mingw_thunk
