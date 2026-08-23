#include <thunk/_common.h>

#include "wcs_coll.h"

namespace mingw_thunk
{
  // Locale argument ignored; the _l twin of wcsxfrm.
  __DEFINE_THUNK(api_ms_win_crt_string_l1_1_0,
                 0,
                 size_t,
                 __cdecl,
                 _wcsxfrm_l,
                 wchar_t *dst,
                 const wchar_t *src,
                 size_t count,
                 _locale_t locale)
  {
    (void)locale;
    return wcoll::xfrm(dst, src, count);
  }
} // namespace mingw_thunk
