#pragma once

// plan-3 §6.1 M14a: the wide collation family — the last ten faces of
// the string api-set, and the last faces of Phase 2.
//
// The one thing that makes this family more than a rename of the narrow
// `strcoll` family is that a `wchar_t` here is a UTF-16 code *unit* and
// not a character.  `wcscmp` orders by unit, so a surrogate pair (whose
// units are 0xD800..0xDFFF) sorts before U+E000 — the wrong side of
// every BMP character above it.  The reference's own C-locale path is
// `wcscmp` and wine's `wcscoll` answers with that order; this layer
// deliberately does not (plan-3 D15, api-set §3.7b): it decodes the
// pairs first and compares code points, so U+E000 < U+1F600 holds.
//
// The return shapes are wine's, and they are not uniform across the
// family — `wcscoll` normalises to -1/0/1 (its CompareStringW path
// subtracts 2), while the three `_wcs?icoll` faces answer the folded
// unit difference.  Both are anchored, so both are kept.
//
// The narrow `strxfrm` this family's `wcsxfrm` mirrors copies exactly
// `count` bytes and pads with NULs when the source is shorter; wine's
// `wcsxfrm` does the same with units.  (M8's `strxfrm` did not pad —
// a divergence found while anchoring this batch and fixed with it, so
// that "strxfrm 决策镜像" is true in the direction the anchor says.)

#include <thunk/u8crt/c_utf8_tables.h>

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <string.h>
#include <wchar.h>

namespace mingw_thunk
{
  namespace wcoll
  {
    // _NLSCMPERROR.  The reference returns it (with EINVAL) for every
    // validation failure in this family, rather than a bare -1.
    constexpr int kNlsCmpError = INT_MAX;

    // One character of a UTF-16 string, pairing surrogates.  An unpaired
    // surrogate is not a character, but it is not the terminator either,
    // so it stands for itself and the walk stays total.  *units is what
    // the character costs in `wchar_t` units -- which is also the unit
    // `_wcsncoll`'s count is measured in.
    inline char32_t next_char(const wchar_t *s, unsigned *units) noexcept
    {
      const unsigned first = static_cast<unsigned>(s[0]);
      if (first >= 0xD800u && first <= 0xDBFFu) {
        const unsigned second = static_cast<unsigned>(s[1]);
        if (second >= 0xDC00u && second <= 0xDFFFu) {
          *units = 2;
          return static_cast<char32_t>(0x10000u + ((first - 0xD800u) << 10) +
                                       (second - 0xDC00u));
        }
      }
      *units = 1;
      return static_cast<char32_t>(first);
    }

    // Lowercase fold of one scalar, the same split the string and
    // multibyte families use: ASCII through the byte table, the rest
    // through the codepoint folder.  Above the BMP there is no fold to
    // apply -- this ABI's `wchar_t` is 16 bits, so the folder cannot
    // name those code points at all.
    inline char32_t fold(char32_t cp) noexcept
    {
      if (cp < 0x80)
        return static_cast<char32_t>(
            i::u8_byte_lower(static_cast<unsigned char>(cp)));
      if (cp > 0xFFFF)
        return cp;
      return static_cast<char32_t>(i::u8_wfold_lower(static_cast<wint_t>(cp)));
    }

    // The shared walk.  `count` is in `wchar_t` units and (size_t)-1 is
    // unbounded; a character whose units do not fit in what is left of
    // the bound ends the comparison, so the bound never splits a pair.
    // `folded` lowercases both sides; `normalized` answers -1/0/1
    // instead of the code point difference.
    inline int compare(const wchar_t *a,
                       const wchar_t *b,
                       size_t count,
                       bool folded,
                       bool normalized) noexcept
    {
      size_t left_a = count;
      size_t left_b = count;
      for (;;) {
        unsigned ua = 0;
        char32_t ca = next_char(a, &ua);
        if (left_a != (size_t)-1 && ua > left_a)
          return 0; // the bound ends inside this character
        unsigned ub = 0;
        char32_t cb = next_char(b, &ub);
        if (left_b != (size_t)-1 && ub > left_b)
          return 0;

        if (folded) {
          ca = fold(ca);
          cb = fold(cb);
        }
        if (ca != cb) {
          if (normalized)
            return ca < cb ? -1 : 1;
          return static_cast<int>(ca) - static_cast<int>(cb);
        }
        if (ca == 0)
          return 0; // both strings ended together

        if (left_a != (size_t)-1) {
          left_a -= ua;
          left_b -= ub;
        }
        a += ua;
        b += ub;
      }
    }

    // The identity collation transform, mirroring the narrow `strxfrm`:
    // copy min(length, count) code units and pad the rest of the window
    // with NULs, answering the source's *full* length either way.  The
    // three validates and their return value (kNlsCmpError, not -1) are
    // the reference's own, in the reference's own order.
    inline size_t xfrm(wchar_t *dst, const wchar_t *src, size_t count) noexcept
    {
      if (count > static_cast<size_t>(INT_MAX)) {
        _set_errno(EINVAL);
        return kNlsCmpError;
      }
      if (!dst && count != 0) {
        _set_errno(EINVAL);
        return kNlsCmpError;
      }
      if (!src) {
        _set_errno(EINVAL);
        return kNlsCmpError;
      }

      const size_t length = wcslen(src);
      if (dst && count) {
        const size_t n = length < count ? length : count;
        memcpy(dst, src, n * sizeof(wchar_t));
        if (n < count)
          memset(dst + n, 0, (count - n) * sizeof(wchar_t));
      }
      return length;
    }
  } // namespace wcoll
} // namespace mingw_thunk
