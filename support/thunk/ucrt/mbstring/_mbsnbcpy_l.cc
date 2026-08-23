#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // Locale argument ignored; the _l twin of _mbsnbcpy.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 unsigned char *,
                 __cdecl,
                 _mbsnbcpy_l,
                 unsigned char *dst,
                 const unsigned char *src,
                 size_t count,
                 _locale_t locale)
  {
    (void)locale;
    if (!count)
      return dst;
    if (!dst || !src) {
      _set_errno(EINVAL);
      return nullptr;
    }
    size_t used = 0;
    mbstring::copy_chars(dst, src, (size_t)-1, count, &used);
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
