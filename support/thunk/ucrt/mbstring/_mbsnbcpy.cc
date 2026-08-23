#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // The byte-counted twin of _mbsncpy: at most count *bytes*, padded out
  // to count bytes, which is strncpy's contract exactly.  The two faces
  // disagree on the same source with the same number, which is the
  // contrast the test matrix pins down.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 unsigned char *,
                 __cdecl,
                 _mbsnbcpy,
                 unsigned char *dst,
                 const unsigned char *src,
                 size_t count)
  {
    if (!count)
      return dst;
    if (!dst || !src) {
      _set_errno(EINVAL);
      return nullptr;
    }
    size_t used = 0;
    mbstring::copy_chars(dst, src, (size_t)-1, count, &used);
    // The reference repairs a tail the *terminator* cut short inside its
    // copy loop, before the padding loop, and so does this: a dud left in
    // place sits between the copied prefix and the padding and makes the
    // destination a string that does not decode.  Its `dst[-2] = 0` names
    // the right offset for a two-byte dud pair and not for a four-byte one,
    // so the offset is read off the source instead.
    if (used) {
      const unsigned char *tail = mbstring::last_char_start(src, used);
      if (mbstring::incomplete_at(tail, src + used)) {
        used = (size_t)(tail - src);
        dst[used] = 0;
      }
    }
    if (used < count)
      memset(dst + used, 0, count - used);
    return dst;
  }
} // namespace mingw_thunk
