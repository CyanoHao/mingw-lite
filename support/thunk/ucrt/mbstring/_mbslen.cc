#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // Characters in the whole string.  The reference shell's own comment
  // settles the unit -- "Find the length of the MBCS string (in
  // characters)" -- and its loop stops before a lead byte the terminator
  // follows, so a truncated tail is not counted.  This is *not* the same
  // function as _mbstrlen below, which validates and reports EILSEQ.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 size_t,
                 __cdecl,
                 _mbslen,
                 const unsigned char *string)
  {
    return mbstring::mbs_len(string);
  }
} // namespace mingw_thunk
