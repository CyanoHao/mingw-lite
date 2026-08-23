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
  // Positional-argument memory variant (plan §M4.2/M4.4, wine-anchored):
  // the legacy bounded shape with the boundary shifted to
  // need <= count for success and a terminator only when need < count.
  // need == count writes exactly count data bytes with NO terminator
  // and still succeeds (wine p_eq3); need > count writes the count-
  // byte prefix, no terminator, and returns -1 (wine p_trunc8).
  __DEFINE_THUNK(api_ms_win_crt_stdio_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 __stdio_common_vsprintf_p,
                 uint64_t options,
                 char *str,
                 size_t count,
                 const char *format,
                 _locale_t locale,
                 va_list arglist)
  {
    (void)locale;

    if (!str || !format) {
      errno = EINVAL;
      return -1;
    }

    i::shell::exp_guard guard(options);

    int need = musl::vsnprintf(nullptr, 0, format, arglist);
    if (need < 0)
      return -1;

    /* room for data plus terminator: direct bounded render */
    if ((size_t)need < count)
      return musl::vsnprintf(str, (size_t)need + 1, format, arglist);

    /* need >= count: exactly count data bytes, no terminator — a
     * direct bounded render would place a NUL at [count-1], so use
     * the scratch-and-copy prefix (M2 legacy bounded precedent) */
    char stackbuf[512];
    char *scratch = stackbuf;
    if ((size_t)need + 1 > sizeof stackbuf) {
      scratch = (char *)malloc((size_t)need + 1);
      if (!scratch) {
        /* degraded fallback: bounded render (count-1 bytes + NUL)
         * with the -1 return; the exact no-terminator layout is not
         * reachable without the scratch */
        musl::vsnprintf(str, count, format, arglist);
        return -1;
      }
    }

    musl::vsnprintf(scratch, (size_t)need + 1, format, arglist);
    memcpy(str, scratch, count);
    if (scratch != stackbuf)
      free(scratch);
    return (size_t)need <= count ? need : -1;
  }
} // namespace mingw_thunk
