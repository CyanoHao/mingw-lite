#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/u8crt/musl.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "printf_shell.h"

namespace mingw_thunk
{
  __DEFINE_THUNK(api_ms_win_crt_stdio_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 __stdio_common_vsprintf,
                 uint64_t options,
                 char *str,
                 size_t len,
                 const char *format,
                 _locale_t locale,
                 va_list arglist)
  {
    (void)locale;

    if (!format) {
      errno = EINVAL;
      return -1;
    }

    i::shell::exp_guard guard(options);

    /* _vscprintf-style measurement: str == NULL, len == 0 */
    if (len == 0)
      return musl::vsnprintf(nullptr, 0, format, arglist);

    if (!str) {
      errno = EINVAL;
      return -1;
    }

    /* C11 snprintf (options bit 0x2): truncation keeps the required
     * length, the terminator is always written — engine semantics
     * exactly (negative results folded to -1 like the wrappers) */
    if (options & 0x2) {
      int r = musl::vsnprintf(str, len, format, arglist);
      return r < 0 ? -1 : r;
    }

    /* legacy unbounded sprintf (bit 0x1, len == (size_t)-1): the
     * string-file cookie treats (size_t)-1 as effectively unbounded
     * and always NUL-terminates */
    if (len == (size_t)-1)
      return musl::vsnprintf(str, (size_t)-1, format, arglist);

    /* legacy _snprintf (bit 0x1, bounded): fits -> direct render
     * (count bytes incl. terminator, returns the length); truncated
     * -> exactly `len` bytes with NO terminator and -1.  A direct
     * bounded render would write len-1 bytes + NUL (one data byte
     * short), so render once into a scratch buffer and copy the exact
     * prefix. */
    int need = musl::vsnprintf(nullptr, 0, format, arglist);
    if (need < 0)
      return -1;
    if ((size_t)need < len)
      return musl::vsnprintf(str, len, format, arglist);

    char stackbuf[512];
    char *scratch = stackbuf;
    if ((size_t)need + 1 > sizeof stackbuf) {
      scratch = (char *)malloc((size_t)need + 1);
      if (!scratch) {
        /* degraded fallback: bounded render (len-1 bytes + NUL) with
         * the legacy -1 return; the exact no-terminator layout is not
         * reachable without the scratch */
        musl::vsnprintf(str, len, format, arglist);
        return -1;
      }
    }

    musl::vsnprintf(scratch, (size_t)need + 1, format, arglist);
    memcpy(str, scratch, len);
    if (scratch != stackbuf)
      free(scratch);
    return -1;
  }
} // namespace mingw_thunk
