#include <thunk/_common.h>

#include "mbs_char.h"

namespace mingw_thunk
{
  // Shift-JIS double-to-single-byte conversion, the reverse of
  // _mbbtombc and identity for the same reason: the reference returns
  // the input unchanged for any codepage but 932.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 unsigned int,
                 __cdecl,
                 _mbctombb,
                 unsigned int c)
  {
    return mbstring::ctombb(c);
  }
} // namespace mingw_thunk
