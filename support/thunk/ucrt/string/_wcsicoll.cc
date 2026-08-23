#include <thunk/_common.h>

#include "wcs_coll.h"

namespace mingw_thunk
{
  // The case-insensitive collation compare.  Both sides are folded by
  // code point and the folded difference is the answer -- not
  // normalised, which is the shape wine's own _wcsicoll has (it goes
  // through the ASCII fold compare, whose answer is a difference).
  // A null operand is EINVAL with _NLSCMPERROR.
  __DEFINE_THUNK(api_ms_win_crt_string_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 _wcsicoll,
                 const wchar_t *s1,
                 const wchar_t *s2)
  {
    if (!s1 || !s2) {
      _set_errno(EINVAL);
      return wcoll::kNlsCmpError;
    }
    return wcoll::compare(s1, s2, (size_t)-1, true, false);
  }
} // namespace mingw_thunk
