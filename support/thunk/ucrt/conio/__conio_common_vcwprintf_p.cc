#include <thunk/_common.h>

#include "conio_channel.h"

#include <stdint.h>

namespace mingw_thunk
{
  // Positional variant of the wide render; %N$ survives the pre-
  // translator and the engine resolves it.
  __DEFINE_THUNK(api_ms_win_crt_conio_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 __conio_common_vcwprintf_p,
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
