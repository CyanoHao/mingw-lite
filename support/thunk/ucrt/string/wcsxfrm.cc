#include <thunk/_common.h>

#include "wcs_coll.h"

namespace mingw_thunk
{
  // The identity collation transform, in units: the caller gets the
  // source copied (up to count units, NUL-padded when it is shorter)
  // and always the source's full length.  The three validates and
  // their kNlsCmpError return are the reference's, and wine's: a null
  // destination is allowed only with a zero count, and a null source
  // is EINVAL.
  __DEFINE_THUNK(api_ms_win_crt_string_l1_1_0,
                 0,
                 size_t,
                 __cdecl,
                 wcsxfrm,
                 wchar_t *dst,
                 const wchar_t *src,
                 size_t count)
  {
    return wcoll::xfrm(dst, src, count);
  }
} // namespace mingw_thunk
