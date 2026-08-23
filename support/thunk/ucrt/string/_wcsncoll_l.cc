#include <thunk/_common.h>

#include "wcs_coll.h"

namespace mingw_thunk
{
  // Locale argument ignored; the _l twin of _wcsncoll.
  __DEFINE_THUNK(api_ms_win_crt_string_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 _wcsncoll_l,
                 const wchar_t *s1,
                 const wchar_t *s2,
                 size_t count,
                 _locale_t locale)
  {
    (void)locale;
    if (count == 0)
      return 0; // the reference answers before it validates anything
    if (!s1 || !s2) {
      _set_errno(EINVAL);
      return wcoll::kNlsCmpError;
    }
    if (count > (size_t)INT_MAX) {
      _set_errno(EINVAL);
      return wcoll::kNlsCmpError;
    }
    return wcoll::compare(s1, s2, count, false, false);
  }
} // namespace mingw_thunk
