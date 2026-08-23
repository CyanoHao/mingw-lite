#include <thunk/_common.h>

#include "mbs_pred.h"

namespace mingw_thunk
{
  // Locale argument ignored; the _l twin of _mbbtype.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 _mbbtype_l,
                 unsigned char c,
                 int ctype,
                 _locale_t locale)
  {
    (void)locale;
    return __crt_mbs::byte_type(c, ctype);
  }
} // namespace mingw_thunk
