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
  // Self-authored shell (no reference *_s.cpp), protocol wine-anchored
  // — same matrix as mbstowcs_s (counting includes the terminator,
  // _TRUNCATE -> STRUNCATE, ERANGE resets, size == 0 -> ret 0).  Wide
  // source: surrogate pairs join into one code point before the UTF-8
  // encode; unpaired surrogates are EILSEQ (engine parity).
  __DEFINE_THUNK(api_ms_win_crt_convert_l1_1_0,
                 0,
                 errno_t,
                 __cdecl,
                 wcstombs_s,
                 size_t *conv,
                 char *dst,
                 size_t size,
                 const wchar_t *src,
                 size_t count)
  {
    using namespace __crt_mbstring;

    const size_t UNBOUNDED = static_cast<size_t>(-1); // == _TRUNCATE

    // Reads one code point from the wide source, joining a surrogate
    // pair; returns consumed units (0 at the terminator, -1 on an
    // unpaired surrogate).
    auto next_code_point = [](const wchar_t *p, size_t left, char32_t *out)
        -> int {
      wchar_t w1 = *p;
      if (w1 == 0)
        return 0;
      if (w1 >= 0xd800 && w1 <= 0xdbff) {
        wchar_t w2 = (left > 1) ? p[1] : 0;
        if (w2 >= 0xdc00 && w2 <= 0xdfff) {
          *out = 0x10000 + ((w1 - 0xd800) << 10) + (w2 - 0xdc00);
          return 2;
        }
        return -1;
      }
      if (w1 >= 0xdc00 && w1 <= 0xdfff)
        return -1;
      *out = w1;
      return 1;
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

      char buf[4];
      mbstate_t st{};
      size_t total = 0;
      const wchar_t *p = src;
      size_t left = count;
      while (left) {
        char32_t cp = 0;
        int units = next_code_point(p, left, &cp);
        if (units < 0) {
          if (conv)
            *conv = 0;
          _set_errno(EILSEQ);
          return EILSEQ;
        }
        if (units == 0)
          break;
        total += __c32rtomb_utf8(buf, cp, &st);
        p += units;
        left -= units;
      }
      if (conv)
        *conv = total + 1;
      return 0;
    }

    if (size == 0) {
      if (conv)
        *conv = 0;
      return 0;
    }

    char *out = dst;
    const size_t bytes = size - 1;
    size_t produced = 0;
    mbstate_t st{};
    char buf[4];
    const wchar_t *p = src;
    size_t left = count;
    const bool truncate = (count == UNBOUNDED);

    for (;;) {
      if (!left) { // count bound reached: terminate cleanly
        *out = 0;
        if (conv)
          *conv = produced + 1;
        return 0;
      }

      char32_t cp = 0;
      int units = next_code_point(p, left, &cp);
      if (units < 0) {
        dst[0] = 0;
        if (conv)
          *conv = 0;
        _set_errno(EILSEQ);
        return EILSEQ;
      }
      if (units == 0) {
        *out = 0;
        if (conv)
          *conv = produced + 1;
        return 0;
      }

      size_t r = __c32rtomb_utf8(buf, cp, &st);
      if (produced + r > bytes) {
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

      for (size_t k = 0; k < r; k++)
        out[k] = buf[k];
      out += r;
      produced += r;
      p += units;
      left -= units;
    }
  }
} // namespace mingw_thunk
