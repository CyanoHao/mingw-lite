#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // First character-aligned occurrence of sub.  The reference compares
  // DBCS units, which is character equality; over UTF-8 byte equality is
  // the same test, so the two agree by construction rather than by luck.
  // An empty sub answers string; a null sub or string is EINVAL.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 const unsigned char *,
                 __cdecl,
                 _mbsstr,
                 const unsigned char *string,
                 const unsigned char *sub)
  {
    return mbstring::mbs_strstr(string, sub);
  }
} // namespace mingw_thunk
