#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // Overwrite the first count *characters* with val, in place.  val is a
  // whole scalar value, the UTF-8 analogue of the DBCS 16-bit code.  A
  // value whose encoded width differs from the character it replaces
  // leaves spaces in the leftover bytes and is EINVAL, which is the
  // reference's "do not orphan lead byte" repair; an unencodable value
  // fills with spaces the same way.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 unsigned char *,
                 __cdecl,
                 _mbsnset,
                 unsigned char *dst,
                 unsigned int val,
                 size_t count)
  {
    if (!count)
      return dst;
    if (!dst) {
      _set_errno(EINVAL);
      return nullptr;
    }
    bool dud = false;
    mbstring::set_chars(dst, val, count, (size_t)-1, &dud);
    if (dud)
      _set_errno(EINVAL);
    return dst;
  }
} // namespace mingw_thunk
