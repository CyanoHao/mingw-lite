#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // Append at most count *characters* of src.  A null dst or src with a
  // nonzero count is EINVAL, but a zero count returns dst without looking
  // at anything, the reference's `if (!cnt) return dst` ahead of its
  // validation section.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 unsigned char *,
                 __cdecl,
                 _mbsncat,
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
    size_t at = 0;
    while (dst[at])
      ++at;
    // a destination ending in an incomplete character drops it rather
    // than gluing the source's first byte onto it: the reference backs
    // the pointer up one byte over the same test, and the start is
    // read back off the string for the reason in _mbsncpy above
    if (at) {
      const unsigned char *dtail = mbstring::last_char_start(dst, at);
      if (mbstring::incomplete_at(dtail, dst + at)) {
        at = (size_t)(dtail - dst);
        dst[at] = 0;
      }
    }
    size_t used = 0;
    size_t chars = 0;
    mbstring::fill_n(dst + at, src, count, &used, &chars);
    dst[at + used] = 0;
    const unsigned char *tail = mbstring::last_char_start(src, used);
    if (used && mbstring::incomplete_at(tail, src + used))
      dst[at + (tail - src)] = 0;
    return dst;
  }
} // namespace mingw_thunk
