#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/u8crt/musl.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(api_ms_win_crt_stdio_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 __stdio_common_vsscanf,
                 uint64_t options,
                 const char *input,
                 size_t length,
                 const char *format,
                 _locale_t locale,
                 va_list arglist)
  {
    if (!input || !format) {
      errno = EINVAL;
      return -1;
    }

    /* _CRT_INTERNAL_SCANF_SECURECRT (0x1): the _s wrappers pass a
     * size_t after every %s/%[/%c pointer, a va_list layout the engine
     * cannot consume — the whole call stays native */
    if (options & 0x1)
      return __ms___stdio_common_vsscanf(
          options, input, length, format, locale, arglist);

    /* length == (size_t)-1 (every wrapper in the reference header):
     * the input is NUL-terminated, the engine scans it directly */
    if (length == (size_t)-1)
      return musl::vsscanf(input, format, arglist);

    /* finite length is a read limit: a NUL inside the bound is
     * equivalent to the terminated case */
    size_t n = 0;
    while (n < length && input[n])
      ++n;
    if (n < length)
      return musl::vsscanf(input, format, arglist);

    /* no NUL within the limit: emulate it with a bounded copy */
    char stackbuf[512];
    char *buf = stackbuf;
    if (length + 1 > sizeof stackbuf) {
      buf = (char *)malloc(length + 1);
      if (!buf) {
        errno = ENOMEM;
        return -1;
      }
    }

    memcpy(buf, input, length);
    buf[length] = '\0';
    int r = musl::vsscanf(buf, format, arglist);
    if (buf != stackbuf)
      free(buf);
    return r;
  }
} // namespace mingw_thunk
