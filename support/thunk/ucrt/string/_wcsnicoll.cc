#include <thunk/_common.h>

#include "wcs_coll.h"

namespace mingw_thunk
{
  // The counted case-insensitive collation compare: the same bound as
  // _wcsncoll with the fold of _wcsicoll, and the folded difference as
  // the answer.
  __DEFINE_THUNK(api_ms_win_crt_string_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 _wcsnicoll,
                 const wchar_t *s1,
                 const wchar_t *s2,
                 size_t count)
  {
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
    return wcoll::compare(s1, s2, count, true, false);
  }
} // namespace mingw_thunk
