#include "../include/uchar.h"
#include "../include/wchar.h"

#include <errno.h>

namespace mingw_thunk
{
  namespace musl
  {
    size_t c16rtomb(char *s, char16_t c16, mbstate_t *ps)
    {
      static unsigned internal_state;
      if (!ps)
        ps = (mbstate_t *)&internal_state;
      unsigned *x = (unsigned *)ps;
      char32_t wc;

      if (!s) {
        if (*x)
          goto ilseq;
        return 1;
      }

      if (!*x && c16 - 0xd800u < 0x400) {
        *x = (c16 - 0xd7c0) << 10;
        return 0;
      }

      if (*x) {
        if (c16 - 0xdc00u >= 0x400)
          goto ilseq;
        wc = *x + c16 - 0xdc00;
        *x = 0;
      } else {
        wc = c16;
      }
      return wcrtomb(s, wc, nullptr);

    ilseq:
      *x = 0;
      errno = EILSEQ;
      return -1;
    }

    size_t c16rtomb(char *s, wchar_t c16, mbstate_t *ps)
    {
      return c16rtomb(s, (char16_t)c16, ps);
    }
  } // namespace musl
} // namespace mingw_thunk
