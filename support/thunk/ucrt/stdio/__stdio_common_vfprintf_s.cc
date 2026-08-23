#include <thunk/_common.h>

#include <stdint.h>
#include <stdio.h>

namespace mingw_thunk
{
  // Secure variant on a stream (plan §M4.2): a stream target has no
  // buffer bound to enforce, and the argument validation is already
  // the plain shell's null guards — pure delegation.
  __DEFINE_THUNK(api_ms_win_crt_stdio_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 __stdio_common_vfprintf_s,
                 uint64_t options,
                 FILE *stream,
                 const char *format,
                 _locale_t locale,
                 va_list arglist)
  {
    return __stdio_common_vfprintf(options, stream, format, locale, arglist);
  }
} // namespace mingw_thunk
