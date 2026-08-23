#include <thunk/_common.h>

#include "mbs_char.h"

namespace mingw_thunk
{
  extern "C" unsigned char _mbcasemap[256];

  // The pointer to the 256-entry ASCII case table.  There is exactly one
  // table: a caller that reads `_mbcasemap` directly and a caller that
  // goes through this pointer must see the same bytes, so this face
  // hands out the address of the data export rather than a copy of it
  // (D7d).
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 unsigned char *,
                 __cdecl,
                 __p__mbcasemap)
  {
    return _mbcasemap;
  }
} // namespace mingw_thunk
