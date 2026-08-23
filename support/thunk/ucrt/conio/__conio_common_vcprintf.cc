#include <thunk/_common.h>

#include "conio_channel.h"

#include <stdint.h>

namespace mingw_thunk
{
  // conio/cprintf.cpp with standard_base: the engine renders the
  // caller's UTF-8 format and the channel widens it once for
  // WriteConsoleW. wine anchor: -1 under a redirected headless run,
  // errno untouched.
  __DEFINE_THUNK(api_ms_win_crt_conio_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 __conio_common_vcprintf,
                 uint64_t options,
                 const char *format,
                 _locale_t locale,
                 va_list arglist)
  {
    (void)locale;

    i::conio::render_out out;
    int err = 0;
    if (!i::conio::render_narrow(options, format, arglist, out, &err)) {
      if (err)
        errno = err;
      return -1;
    }

    return i::conio::emit(out);
  }
} // namespace mingw_thunk
