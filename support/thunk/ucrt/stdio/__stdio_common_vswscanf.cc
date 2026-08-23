#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/u8crt/musl.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include "wfmt_translate.h"

namespace mingw_thunk
{
  __DEFINE_THUNK(api_ms_win_crt_stdio_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 __stdio_common_vswscanf,
                 uint64_t options,
                 const wchar_t *input,
                 size_t length,
                 const wchar_t *format,
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
      return __ms___stdio_common_vswscanf(
          options, input, length, format, locale, arglist);

    i::shell::wfmt_holder nfmt(format, true, false);
    if (!nfmt.ok) {
      errno = ENOMEM;
      return -1;
    }

    /* length == (size_t)-1 (every wrapper in the product header): the
     * input is NUL-terminated; a finite length stops at the first NUL
     * inside the bound (narrow-side equivalence, plan M2-c) */
    size_t n =
        (length == (size_t)-1) ? wcslen(input) : wcsnlen(input, length);

    /* UTF-16 -> UTF-8 for the engine: every unit is at most 3 bytes
     * (lone surrogates become U+FFFD, plan M3.6-7) */
    char stackbuf[512];
    char *buf = stackbuf;
    if (3 * n + 1 > sizeof stackbuf) {
      buf = (char *)malloc(3 * n + 1);
      if (!buf) {
        errno = ENOMEM;
        return -1;
      }
    }

    i::shell::u16_to_u8(input, n, buf);
    int r = musl::vsscanf(buf, nfmt.nfmt, arglist);
    if (buf != stackbuf)
      free(buf);
    return r;
  }
} // namespace mingw_thunk
