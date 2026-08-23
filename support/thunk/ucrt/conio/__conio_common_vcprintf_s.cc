#include <thunk/_common.h>

#include "conio_channel.h"

#include <stdint.h>

namespace mingw_thunk
{
  // Secure variant. A console has no buffer bound to enforce, and the
  // argument validation is already the plain shell's null gate, so the
  // body is the plain routing (M4.2 precedent).
  __DEFINE_THUNK(api_ms_win_crt_conio_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 __conio_common_vcprintf_s,
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
