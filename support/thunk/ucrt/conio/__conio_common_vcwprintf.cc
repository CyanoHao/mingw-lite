#include <thunk/_common.h>

#include "conio_channel.h"

#include <stdint.h>

namespace mingw_thunk
{
  // conio/cprintf.cpp with standard_base, wide family: the M3 pre-
  // translator turns the caller's wide format into the engine's narrow
  // one with the SAME va_list (0x4 selects the legacy wide conventions),
  // then the render is the narrow path widened back out.
  __DEFINE_THUNK(api_ms_win_crt_conio_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 __conio_common_vcwprintf,
                 uint64_t options,
                 const wchar_t *format,
                 _locale_t locale,
                 va_list arglist)
  {
    (void)locale;

    i::conio::render_out out;
    int err = 0;
    if (!i::conio::render_wide(options, format, arglist, out, &err)) {
      if (err)
        errno = err;
      return -1;
    }

    return i::conio::emit(out);
  }
} // namespace mingw_thunk
