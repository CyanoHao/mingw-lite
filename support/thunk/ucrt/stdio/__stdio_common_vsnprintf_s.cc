#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/u8crt/musl.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>

#include "printf_shell.h"

namespace mingw_thunk
{
  __DEFINE_THUNK(api_ms_win_crt_stdio_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 __stdio_common_vsnprintf_s,
                 uint64_t options,
                 char *str,
                 size_t size,
                 size_t maxcount,
                 const char *format,
                 _locale_t locale,
                 va_list arglist)
  {
    (void)locale;

    if (!format) {
      errno = EINVAL;
      return -1;
    }

    /* _TRUNCATE ((size_t)-1) or a roomy maxcount bound the render by
     * the buffer; a tighter maxcount bounds it by maxcount+1 (maxcount
     * characters + terminator).  Truncation returns -1 with the
     * terminator guaranteed. */
    size_t cap =
        (maxcount == (size_t)-1 || maxcount >= size) ? size : maxcount + 1;

    if (cap == 0 || !str) {
      errno = EINVAL;
      return -1;
    }

    i::shell::exp_guard guard(options);

    int r = musl::vsnprintf(str, cap, format, arglist);
    if (r < 0)
      return -1;
    return (size_t)r < cap ? r : -1;
  }
} // namespace mingw_thunk
