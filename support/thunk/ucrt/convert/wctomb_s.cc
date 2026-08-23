#include <thunk/_common.h>
#include <thunk/u8crt/mbstring.h>

#include <errno.h>
#include <stdlib.h>
#include <wchar.h>

namespace mingw_thunk
{
  // Self-authored shell, wine-anchored: null buffer or zero size ->
  // ERANGE with *len = -1; success -> rc 0 and *len = stored bytes
  // (L'\0' stores exactly one NUL byte); a lone surrogate (single
  // wchar input can never form a pair) is EILSEQ per the M5 wctomb
  // decision; convertible-but-too-large is ERANGE.
  __DEFINE_THUNK(api_ms_win_crt_convert_l1_1_0,
                 0,
                 errno_t,
                 __cdecl,
                 wctomb_s,
                 int *size_converted,
                 char *s,
                 size_t size,
                 wchar_t wc)
  {
    using namespace __crt_mbstring;

    if (!s || size == 0) {
      if (size_converted)
        *size_converted = -1;
      _set_errno(ERANGE);
      return ERANGE;
    }

    if (wc >= 0xd800 && wc <= 0xdfff) {
      if (size_converted)
        *size_converted = -1;
      s[0] = 0;
      _set_errno(EILSEQ);
      return EILSEQ;
    }

    char buf[4];
    mbstate_t st{};
    size_t r = __c32rtomb_utf8(buf, static_cast<char32_t>(wc), &st);
    if (r == INVALID || r == 0) {
      if (size_converted)
        *size_converted = -1;
      s[0] = 0;
      _set_errno(EILSEQ);
      return EILSEQ;
    }
    if (r > size) {
      if (size_converted)
        *size_converted = -1;
      s[0] = 0;
      _set_errno(ERANGE);
      return ERANGE;
    }

    for (size_t k = 0; k < r; k++)
      s[k] = buf[k];
    if (size_converted)
      *size_converted = static_cast<int>(r);
    return 0;
  }
} // namespace mingw_thunk
