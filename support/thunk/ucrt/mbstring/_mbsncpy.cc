#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // Copy at most count *characters* and pad with NULs out to count
  // characters, the reference's leftover-counter loop.  A character is
  // never split, so the count can only under- or exactly-fill; the
  // reference instead writes the trail byte first and can overrun the
  // buffer by one byte there.  A truncated source tail zeroes its own
  // bytes, the reference's `dst[-2] = 0` repair widened to every UTF-8
  // width (D9w).
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 unsigned char *,
                 __cdecl,
                 _mbsncpy,
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
    size_t chars = 0;
    mbstring::fill_n(dst, src, count, &used, &chars);
    // A cut tail is located by reading its start back off the source,
    // never by subtracting the lead byte's declared width: a sequence
    // the terminator cut is shorter than that width, so the
    // subtraction is negative.  See mbstring::copy_tail_dud.
    const unsigned char *tail = mbstring::last_char_start(src, used);
    if (used && mbstring::incomplete_at(tail, src + used)) {
      used = (size_t)(tail - src);
      dst[used] = 0;
    }
    if (used < count)
      memset(dst + used, 0, count - chars);
    return dst;
  }
} // namespace mingw_thunk
