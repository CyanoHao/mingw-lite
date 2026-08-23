#include <thunk/_common.h>
#include <thunk/u8crt/mbstring.h>

#include <errno.h>
#include <stdlib.h>
#include <wchar.h>

#ifndef STRUNCATE
#define STRUNCATE 80
#endif

namespace mingw_thunk
{
  // Self-authored shell (reference/ucrt/convert has no *_s.cpp —
  // plan-2 M7), protocol wine-anchored on native ucrtbase:
  //   * counting mode (dst == nullptr && size == 0): ret 0, *conv =
  //     wide chars needed INCLUDING the terminator; count == _TRUNCATE
  //     accepted
  //   * dst == nullptr && size != 0            -> EINVAL
  //   * dst != nullptr && size == 0            -> ret 0, *conv = 0
  //   * src == nullptr                         -> EINVAL
  //   * too-small dst                          -> ERANGE, dst[0] = 0,
  //                                               *conv = 0
  //   * count == _TRUNCATE with truncation     -> STRUNCATE,
  //                                               *conv = fits + 1
  //   * success                                -> *conv = converted + 1
  // UTF-8 divergences kept on purpose: EILSEQ replaces native ACP
  // unconvertible errors; ERANGE zeroes *conv where wine leaves the
  // stale caller value (mbstowcs_s anchor showed both shapes — the
  // C11-consistent one is kept).
  __DEFINE_THUNK(api_ms_win_crt_convert_l1_1_0,
                 0,
                 errno_t,
                 __cdecl,
                 mbstowcs_s,
                 size_t *conv,
                 wchar_t *dst,
                 size_t size,
                 const char *src,
                 size_t count)
  {
    using namespace __crt_mbstring;

    const size_t UNBOUNDED = static_cast<size_t>(-1); // == _TRUNCATE

    // Bytes readable without crossing the NUL or the count bound
    // (engine compute_available shape, bounded).
    auto readable = [](const char *s, size_t bound) -> size_t {
      if (s[0] == '\0')
        return 1;
      if (bound == 1)
        return 1;
      if (s[1] == '\0')
        return 2;
      if (bound == 2)
        return 2;
      if (s[2] == '\0')
        return 3;
      if (bound == 3)
        return 3;
      return 4;
    };

    if (!src) {
      if (conv)
        *conv = 0;
      if (dst && size)
        dst[0] = 0;
      _set_errno(EINVAL);
      return EINVAL;
    }

    if (!dst) {
      if (size != 0) {
        if (conv)
          *conv = 0;
        _set_errno(EINVAL);
        return EINVAL;
      }

      size_t wide = 0;
      const char *p = src;
      size_t left = count;
      mbstate_t st{};
      while (left) {
        char32_t c32 = 0;
        size_t r = __mbrtoc32_utf8(&c32, p, readable(p, left), &st);
        if (r == INVALID) {
          if (conv)
            *conv = 0;
          _set_errno(EILSEQ);
          return EILSEQ;
        }
        if (r == INCOMPLETE || r == 0)
          break; // count cut mid-sequence / terminator reached
        p += r;
        left -= r;
        wide += (r == 4) ? 2 : 1;
      }
      if (conv)
        *conv = wide + 1;
      return 0;
    }

    if (size == 0) {
      if (conv)
        *conv = 0;
      return 0;
    }

    wchar_t *out = dst;
    const size_t slots = size - 1;
    size_t produced = 0;
    mbstate_t st{};
    const char *p = src;
    size_t left = count;
    const bool truncate = (count == UNBOUNDED);

    for (;;) {
      if (!left) { // count bound reached: terminate cleanly
        *out = 0;
        if (conv)
          *conv = produced + 1;
        return 0;
      }

      char32_t c32 = 0;
      size_t r = __mbrtoc32_utf8(&c32, p, readable(p, left), &st);
      if (r == INVALID) {
        dst[0] = 0;
        if (conv)
          *conv = 0;
        _set_errno(EILSEQ);
        return EILSEQ;
      }
      if (r == INCOMPLETE || r == 0) {
        *out = 0;
        if (conv)
          *conv = produced + 1;
        return 0;
      }

      const size_t units = (r == 4) ? 2 : 1;
      if (produced + units > slots) {
        if (truncate) {
          *out = 0;
          if (conv)
            *conv = produced + 1;
          return STRUNCATE;
        }
        dst[0] = 0;
        if (conv)
          *conv = 0;
        _set_errno(ERANGE);
        return ERANGE;
      }

      if (units == 2) {
        c32 -= 0x10000;
        *out++ = static_cast<wchar_t>((c32 >> 10) | 0xd800);
        *out++ = static_cast<wchar_t>((c32 & 0x03ff) | 0xdc00);
        produced += 2;
      } else {
        *out++ = static_cast<wchar_t>(c32);
        produced += 1;
      }
      p += r;
      left -= r;
    }
  }
} // namespace mingw_thunk
