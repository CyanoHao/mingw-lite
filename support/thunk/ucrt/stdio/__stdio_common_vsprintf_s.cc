#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/u8crt/musl.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>

#include "printf_shell.h"

namespace mingw_thunk
{
  // Secure memory variant (plan §M4.2/M4.4, wine-anchored): identical
  // to a C11 render with cap = count + 1 plus the truncation folded
  // to -1.  The engine writes min(need, count) data bytes and the
  // terminator at [min] — including the boundary shapes wine
  // exhibited (need == count succeeds with the NUL at [count];
  // count == 0 writes only the terminator at [0] and returns -1).
  __DEFINE_THUNK(api_ms_win_crt_stdio_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 __stdio_common_vsprintf_s,
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

    size_t cap = count + 1;
    if (cap == 0) /* count == SIZE_MAX: effectively unbounded */
      cap = count;

    int r = musl::vsnprintf(str, cap, format, arglist);
    if (r < 0)
      return -1;
    return (size_t)r <= count ? r : -1;
  }
} // namespace mingw_thunk
