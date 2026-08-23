#include <thunk/_common.h>

#include "wcs_coll.h"

namespace mingw_thunk
{
  // The counted collation compare.  The count is in `wchar_t` units,
  // and a character whose units do not fit in what is left of the
  // bound ends the comparison rather than being split -- so the bound
  // is the reference's `wcsncmp` bound and the order is still by code
  // point.  A zero count answers 0 before the pointers are looked at
  // (the reference returns before its validation block); a null
  // operand or a count above INT_MAX is EINVAL with _NLSCMPERROR.
  // The return is the difference, which is wine's shape for this
  // face -- only `wcscoll` normalises.
  __DEFINE_THUNK(api_ms_win_crt_string_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 _wcsncoll,
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
    return wcoll::compare(s1, s2, count, false, false);
  }
} // namespace mingw_thunk
