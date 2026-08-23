#include <thunk/_common.h>

#include "mbs_char.h"

namespace mingw_thunk
{
  // The pointer to the character-type table.  The table holds bit
  // masks (_SBUP/_SBLOW for ASCII letters, _M1/_M2 for the UTF-8 lead
  // and trail ranges), not the _MBC_* values the test family returns
  // (D7d).  It is a pointer rather than a plain array because that is
  // the shape the export has; there is no bare _mbctype data export.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 unsigned char *,
                 __cdecl,
                 __p__mbctype)
  {
    return const_cast<unsigned char *>(mbstring::mbctype_table());
  }
} // namespace mingw_thunk
