#include <thunk/_common.h>

#include "wcs_coll.h"

namespace mingw_thunk
{
  // Compare two wide strings in collation order.  This layer's
  // collation is code point order, so the comparison cannot be
  // `wcscmp`: a surrogate pair is two units in 0xD800..0xDFFF and would
  // sort before every BMP character above it.  The pairs are decoded
  // first, and the answer is normalised to -1/0/1 -- wine's shape,
  // whose CompareStringW path subtracts 2 from its own 1/2/3.  A null
  // operand is EINVAL with _NLSCMPERROR, the reference's validation.
  // (wine divergence, D15: wine orders the pair by its units, so it
  // answers U+1F600 < U+E000; this layer answers U+E000 < U+1F600.)
  __DEFINE_THUNK(api_ms_win_crt_string_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 wcscoll,
                 const wchar_t *s1,
                 const wchar_t *s2)
  {
    if (!s1 || !s2) {
      _set_errno(EINVAL);
      return wcoll::kNlsCmpError;
    }
    return wcoll::compare(s1, s2, (size_t)-1, false, true);
  }
} // namespace mingw_thunk
