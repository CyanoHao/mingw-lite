#include <thunk/_common.h>

#include "conio_channel.h"

#include <stdint.h>

namespace mingw_thunk
{
  // conio/cscanf.cpp with the console input adapter: one cooked console
  // line is narrowed and handed to the engine. wine does not export an
  // implementation, so the readable contract is the reference plus the
  // M3 scanf rewrite (bare %s/%c/%[ widen, %hs/%hc/%h[ stay narrow).
  __DEFINE_THUNK(api_ms_win_crt_conio_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 __conio_common_vcscanf,
                 uint64_t options,
                 const char *format,
                 _locale_t locale,
                 va_list arglist)
  {
    (void)locale;
    (void)options; // the console family has no secure entry, so bit 0x1
                   // never arrives and the scanf rewrite is the only
                   // thing options can select

    if (format == nullptr) {
      errno = EINVAL;
      return -1;
    }

    return i::conio::scan_line_narrow(format, arglist);
  }
} // namespace mingw_thunk
