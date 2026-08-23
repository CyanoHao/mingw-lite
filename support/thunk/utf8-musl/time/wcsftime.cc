/* musl src/time/wcsftime.c port (plan-2 M9 §4.3): the musl shape —
 * the SAME __strftime_fmt_1 renders each fragment, converted to
 * UTF-16 here; format literals are copied wide directly.  Same
 * adaptations as strftime.cc (no langinfo, tz-state %z/%Z/%s,
 * hand-rolled digit parse); the per-fragment mbstowcs is a local
 * UTF-8 -> UTF-16 converter and wmemcpy is an open loop, so the
 * engine has no CRT wide-string coupling.  musl quirks kept
 * verbatim: the >= clamp (vs the narrow engine's >) and the
 * simpler wide width branch. */

#include "../internal/time_impl.h"

#include <thunk/u8crt/musl.h>

#include <stddef.h>
#include <stdint.h>

namespace mingw_thunk
{
  namespace musl
  {
    namespace
    {
      inline bool wis_digit(wchar_t c)
      {
        return c >= L'0' && c <= L'9';
      }

      /* UTF-8 -> UTF-16 (surrogate pairs); (size_t)-1 on malformed
       * input, mirroring the mbstowcs contract */
      size_t u8_to_ws(wchar_t *dst, const char *src, size_t cap)
      {
        size_t l = 0;
        const unsigned char *p = (const unsigned char *)src;
        while (*p) {
          uint32_t cp;
          unsigned len;
          if (*p < 0x80) {
            cp = *p;
            len = 1;
          } else if ((*p & 0xE0) == 0xC0) {
            cp = *p & 0x1F;
            len = 2;
          } else if ((*p & 0xF0) == 0xE0) {
            cp = *p & 0x0F;
            len = 3;
          } else if ((*p & 0xF8) == 0xF0) {
            cp = *p & 0x07;
            len = 4;
          } else {
            return (size_t)-1;
          }
          for (unsigned i = 1; i < len; i++) {
            if ((p[i] & 0xC0) != 0x80)
              return (size_t)-1;
            cp = (cp << 6) | (p[i] & 0x3F);
          }
          /* reject overlong forms and surrogates (defensive: the
           * engine renders valid UTF-8) */
          if (cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF) ||
              (len == 2 && cp < 0x80) || (len == 3 && cp < 0x800) ||
              (len == 4 && cp < 0x10000))
            return (size_t)-1;
          p += len;
          size_t need = cp >= 0x10000 ? 2 : 1;
          if (l + need > cap)
            return (size_t)-1;
          if (cp >= 0x10000) {
            cp -= 0x10000;
            dst[l++] = (wchar_t)(0xD800 + (cp >> 10));
            dst[l++] = (wchar_t)(0xDC00 + (cp & 0x3FF));
          } else {
            dst[l++] = (wchar_t)cp;
          }
        }
        return l;
      }
    } // namespace

    size_t wcsftime(wchar_t *s, size_t n, const wchar_t *f,
                    const struct tm *tm)
    {
      size_t l, k;
      char buf[100];
      wchar_t wbuf[100];
      const char *t_mb;
      const wchar_t *t;
      int pad, plus;
      unsigned long width;
      for (l = 0; l < n; f++) {
        if (!*f) {
          s[l] = 0;
          return l;
        }
        if (*f != '%') {
          s[l++] = *f;
          continue;
        }
        f++;
        pad = 0;
        if (*f == L'-' || *f == L'_' || *f == L'0')
          pad = *f++;
        if ((plus = (*f == L'+')))
          f++;
        width = 0;
        int has_w = 0;
        if (wis_digit(*f)) {
          has_w = 1;
          while (wis_digit(*f)) {
            width = width * 10 + (unsigned long)(*f++ - L'0');
            if (width > 0x10000000UL)
              width = 0x10000000UL;
          }
        }
        const wchar_t *p = f;
        if (*p == L'C' || *p == L'F' || *p == L'G' || *p == L'Y') {
          if (!width && has_w)
            width = 1;
        } else {
          width = 0;
        }
        if (*f == L'E' || *f == L'O')
          f++;
        t_mb = __strftime_fmt_1(&buf, &k, (int)*f, tm, pad);
        if (!t_mb)
          break;
        k = u8_to_ws(wbuf, t_mb, sizeof wbuf / sizeof *wbuf);
        if (k == (size_t)-1)
          return 0;
        t = wbuf;
        if (width) {
          for (; *t == L'+' || *t == L'-' || (*t == L'0' && t[1]);
               t++, k--)
            ;
          width--;
          if (plus && tm->tm_year >= 10000 - 1900)
            s[l++] = L'+';
          else if (tm->tm_year < -1900)
            s[l++] = L'-';
          else
            width++;
          for (; width > k && l < n; width--)
            s[l++] = L'0';
        }
        if (k >= n - l)
          k = n - l;
        for (size_t i = 0; i < k; i++)
          s[l + i] = t[i];
        l += k;
      }
      if (n) {
        if (l == n)
          l = n - 1;
        s[l] = 0;
      }
      return 0;
    }
  } // namespace musl
} // namespace mingw_thunk
