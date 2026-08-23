#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // Lowercase-folded code-point order, whole string.  The reference
  // folds this one face with LCMAP_UPPERCASE and its counted
  // siblings with _mbbtolower; the two agree on any fold that maps a
  // case pair onto one value, and lowercase is what plan-3 5.1 fixes
  // for the family.  This is a codepoint fold, unlike M8's _stricmp
  // byte fold -- the layering is deliberate and documented.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 _mbsicmp,
                 const unsigned char *s1,
                 const unsigned char *s2)
  {
    return mbstring::mbs_compare(
        s1, s2, 0, mbstring::CMP_FOLD, mbstring::BND_STR);
  }
} // namespace mingw_thunk
