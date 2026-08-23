#pragma once

#include <errno.h>
#include <stdint.h>

namespace mingw_thunk
{
  namespace musl
  {
    int wctomb(char *, char32_t);

    /* Convert the next code point of the UTF-16 stream ws to UTF-8.
     * Surrogate pairs are combined into one code point; a lone surrogate
     * fails (EILSEQ decision, see wctomb/wcrtomb). Returns the number of
     * bytes written to mb (1-4) or -1; ws is advanced only on success. */
    inline int ws_wctomb(char mb[4], wchar_t *&ws) noexcept
    {
      const wchar_t c0 = ws[0];
      char32_t cp;
      int adv = 1;

      if (c0 >= 0xd800 && c0 < 0xdc00) {
        const wchar_t c1 = ws[1];
        if (!(c1 >= 0xdc00 && c1 < 0xe000)) {
          errno = EILSEQ;
          return -1;
        }
        cp = 0x10000 + ((char32_t)(c0 - 0xd800) << 10) + (c1 - 0xdc00);
        adv = 2;
      } else if (c0 >= 0xdc00 && c0 < 0xe000) {
        errno = EILSEQ;
        return -1;
      } else {
        cp = c0;
      }

      int l = wctomb(mb, cp);
      if (l >= 0)
        ws += adv;
      return l;
    }
  } // namespace musl
} // namespace mingw_thunk
