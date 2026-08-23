#pragma once

// M11 _ecvt/_fcvt/_gcvt cores (plan-3 §3.3): render through the in-tree
// musl::vsnprintf engine (decimal point '.', frozen) and do the digit
// surgery afterwards.  Protocol matrices are wine-anchored (plan-3 §3.7,
// safe C/C2/C3 groups + one-shot capacity probes).
//
// Validation shapes (wine beats reference where they disagree — M10 r6
// precedent):
//  * null buffer / null out-param -> rc EINVAL, errno UNTOUCHED
//  * ecvt_s: cap == 0 -> rc ERANGE, errno ERANGE, buffer untouched
//    (reference resets the buffer first; wine does not)
//  * ecvt_s: cap < count + 2 -> rc ERANGE, errno ERANGE, buffer and
//    decpt/sign untouched
//  * fcvt_s: cap < strlen(digits) + 1 -> rc ERANGE, errno ERANGE
//    (wine ucrtbase dies on the probed insufficient-capacity case — an
//    invalid-parameter shell bug we do not replicate; graceful divergence)
//  * gcvt_s: cap == 0 -> rc ERANGE, buffer untouched; cap <= precision
//    or rendered string does not fit -> rc ERANGE, errno ERANGE, buffer
//    cleared.  (The native E-format slack window above strlen+1 is not
//    replicated — edge divergence, plan-3 §3.7.)

#include <thunk/_common.h>
#include <thunk/buffer.h>
#include <thunk/u8crt/musl.h>

#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

namespace mingw_thunk
{
  namespace u8crt_convert
  {
    // builds "%.N<conv>" (no star — vararg order is value-only) and
    // renders; the double value rides in as the sole variadic argument.
    // Returns the musl vsnprintf length (required size even when
    // truncated).
    inline int
    render_fp(char *s, size_t n, int prec, char conv, ...) noexcept
    {
      unsigned u = prec > 0 ? (unsigned)prec : 0u;
      char fmt[24];
      char *f = fmt;
      *f++ = '%';
      *f++ = '.';
      char digits[12];
      int l = 0;
      do {
        digits[l++] = char('0' + u % 10);
        u /= 10;
      } while (u);
      while (l)
        *f++ = digits[--l];
      *f++ = conv;
      *f = 0;

      va_list ap;
      va_start(ap, conv);
      const int r = musl::vsnprintf(s, n, fmt, ap);
      va_end(ap);
      return r;
    }

    // Renders and returns the string in `out` (NUL-terminated).  False on
    // allocation failure only.
    inline bool
    render_to_buffer(d::buffer<128> &out, int prec, char conv,
                     double v) noexcept
    {
      const int needed = render_fp(nullptr, 0, prec, conv, v);
      if (needed < 0)
        return false;
      if (!out.resize(size_t(needed) + 1))
        return false;
      render_fp(out.data(), out.size() + 1, prec, conv, v);
      return true;
    }

    // ---- _ecvt / _ecvt_s ------------------------------------------------
    //
    // "%.<count-1>e" gives exactly `count` correctly rounded significant
    // digits; decpt = exponent + 1 (0.999 -> "9.99e-01" -> "999", decpt 0;
    // 9.999/2 -> "1.0e+01" -> "10", decpt 2).  Zero is special-cased to
    // decpt 0 (wine: _ecvt(0,5) = "00000" dec=0).
    inline errno_t
    ecvt_core(char *buf,
              size_t cap,
              double value,
              int count,
              int *decpt,
              int *sign) noexcept
    {
      if (!buf)
        return EINVAL;
      if (cap == 0) {
        errno = ERANGE;
        return ERANGE;
      }
      if (!decpt || !sign)
        return EINVAL;
      if (cap < size_t(count > 0 ? count : 0) + 2) {
        errno = ERANGE;
        return ERANGE;
      }

      d::buffer<128> out;
      if (!render_to_buffer(out, count > 0 ? count - 1 : 0, 'e', value))
        return ENOMEM;

      const char *s = out.data();
      const bool negative = (*s == '-');
      if (negative)
        ++s;
      *sign = negative ? 1 : 0;

      int exponent = 0;
      const char *e = strchr(s, 'e');
      if (e) {
        int esign = 1;
        ++e;
        if (*e == '+') {
          ++e;
        } else if (*e == '-') {
          esign = -1;
          ++e;
        }
        while (*e >= '0' && *e <= '9')
          exponent = exponent * 10 + (*e++ - '0');
        exponent *= esign;
      }
      *decpt = (value == 0.0) ? 0 : exponent + 1;

      // digits: every rendered char except '.', up to `count`, zero-padded
      // (%.<n-1>e always yields exactly n digits — the pad is defensive);
      // count == 0 renders an empty string (wine: _ecvt(123.456,0) = "").
      const int want = count > 0 ? count : 0;
      int filled = 0;
      for (const char *d = s; filled < want && *d && *d != 'e'; ++d) {
        if (*d == '.')
          continue;
        buf[filled++] = *d;
      }
      while (filled < want)
        buf[filled++] = '0';
      buf[want] = 0;
      return 0;
    }

    // ---- _fcvt / _fcvt_s ------------------------------------------------
    //
    // "%.<count>f", drop the '.', then strip leading zeros from the whole
    // digit run (fltout-equivalent): 0.005/2 -> "0.01" -> "1" dec=-1;
    // 1e-8/10 -> "100" dec=-7; zero is kept verbatim ("000", dec 0).
    inline errno_t
    fcvt_core(char *buf,
              size_t cap,
              double value,
              int count,
              int *decpt,
              int *sign) noexcept
    {
      if (!buf)
        return EINVAL;
      if (cap == 0) {
        errno = ERANGE;
        return ERANGE;
      }
      if (!decpt || !sign)
        return EINVAL;

      d::buffer<128> out;
      if (!render_to_buffer(out, count > 0 ? count : 0, 'f', value))
        return ENOMEM;

      const char *s = out.data();
      const bool negative = (*s == '-');
      if (negative)
        ++s;
      *sign = negative ? 1 : 0;

      // zero (incl. -0.0): `count` zeros, decpt 0 (wine: _fcvt(0,3) =
      // "000" dec=0; _fcvt(-0,2) = "00" dec=0 sign=1 — kept verbatim, no
      // leading-zero strip)
      if (value == 0.0) {
        const size_t want = count > 0 ? size_t(count) : 0;
        if (want + 1 > cap) {
          errno = ERANGE;
          return ERANGE;
        }
        memset(buf, '0', want);
        buf[want] = 0;
        *decpt = 0;
        return 0;
      }

      const char *dot = strchr(s, '.');
      const size_t int_len = dot ? size_t(dot - s) : strlen(s);
      const size_t frac_len = dot ? strlen(dot + 1) : 0;
      const size_t total = int_len + frac_len;

      // Native shape (fltout-equivalent): the emitted digit run carries no
      // leading zeros — 0.005/2 -> "0.01" -> "1" dec=-1; 1e-8/10 -> "100"
      // dec=-7; zero itself is kept verbatim ("000"/dec 0, handled above).
      // k = leading zeros of the dot-removed sequence.
      size_t k = 0;
      for (const char *p = s; *p && k < total; ++p) {
        if (*p == '.')
          continue;
        if (*p != '0')
          break;
        ++k;
      }

      if (k == total) {
        // value != 0 but rounds to zero at this digit count (unprobed
        // native corner — kept as a single "0", plan-3 §3.7 note)
        if (cap < 2) {
          errno = ERANGE;
          return ERANGE;
        }
        buf[0] = '0';
        buf[1] = 0;
        *decpt = 0;
        return 0;
      }

      *decpt = int(int_len) - int(k);

      const size_t int_zeros = k < int_len ? k : int_len;
      const size_t frac_zeros = k - int_zeros;
      const size_t int_kept = int_len - int_zeros;
      const size_t frac_kept = frac_len - frac_zeros;
      if (int_kept + frac_kept + 1 > cap) {
        errno = ERANGE;
        return ERANGE;
      }

      size_t n = 0;
      if (int_kept) {
        memcpy(buf, s + int_zeros, int_kept);
        n = int_kept;
      }
      if (frac_kept) {
        memcpy(buf + n, dot + 1 + frac_zeros, frac_kept);
        n += frac_kept;
      }
      buf[n] = 0;
      return 0;
    }

    // ---- _gcvt / _gcvt_s ------------------------------------------------
    //
    // Native shape (wine C3/C6): exactly printf %.*g — P significant
    // digits, E style when exponent < -4 or >= P, two-digit exponents,
    // trailing zeros stripped ("1e-05", "1.23457e+06", "0.0001", "0").
    inline errno_t
    gcvt_core(char *buf, size_t cap, double value, int precision) noexcept
    {
      if (!buf)
        return EINVAL;
      if (cap == 0) {
        errno = ERANGE;
        return ERANGE; // r10: buffer untouched at cap == 0
      }
      if (size_t(precision < 0 ? -1 : precision) >= cap) {
        buf[0] = 0; // reference _RESET_STRING (wine probe2: cleared)
        errno = ERANGE;
        return ERANGE;
      }

      d::buffer<128> out;
      if (!render_to_buffer(out, precision, 'g', value))
        return ENOMEM;

      const size_t len = strlen(out.data());
      if (len + 1 > cap) {
        buf[0] = 0;
        errno = ERANGE;
        return ERANGE;
      }
      memcpy(buf, out.data(), len + 1);
      return 0;
    }
  } // namespace u8crt_convert
} // namespace mingw_thunk
