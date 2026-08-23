#include <thunk/_common.h>
#include <thunk/string.h>
#include <thunk/u8crt/c_utf8_tables.h>

namespace mingw_thunk
{
  // C.UTF-8 collation == code point order == byte order: identity
  // transform is correct (wine anchors: n == 0 -> strlen, truncated
  // copy of exactly count bytes, NUL-padded to count when the source is
  // shorter, returns full length).  The padding is strncpy's, which is
  // the reference's C-locale path verbatim -- and it was missing here
  // until M14 anchored the wide twin and found the same shape.
  __DEFINE_THUNK(api_ms_win_crt_string_l1_1_0,
                 0,
                 size_t,
                 __cdecl,
                 strxfrm,
                 char *dst,
                 const char *src,
                 size_t count)
  {
    size_t length = c::strlen(src);
    if (dst && count) {
      size_t n = length < count ? length : count;
      for (size_t k = 0; k < n; k++)
        dst[k] = src[k];
      for (size_t k = n; k < count; k++)
        dst[k] = 0;
    }
    return length;
  }
} // namespace mingw_thunk
