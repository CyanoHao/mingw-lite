#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // Code-point order over at most count *bytes* -- the "nb" half.
  // A character the bound would split reads as the terminator, which
  // is the reference's "naked lead" case, so a bound landing inside
  // a character reports equality rather than a mid-character
  // mismatch.  Each side keeps its own byte tally, because the
  // reference's single shared countdown lets the two sides run out
  // at different points once a bound lands inside a character.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 _mbsnbcmp,
                 const unsigned char *s1,
                 const unsigned char *s2,
                 size_t count)
  {
    return mbstring::mbs_compare(
        s1, s2, count, mbstring::CMP_RAW, mbstring::BND_BYTES);
  }
} // namespace mingw_thunk
