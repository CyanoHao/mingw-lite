#include <thunk/_common.h>

#include "conio_channel.h"

#include <stdint.h>

namespace mingw_thunk
{
  // Wide input half: the format is pre-translated for the engine and the
  // same line is scanned, so the caller's wchar_t* destinations are
  // filled exactly as the stdio vswscanf face fills them.
  __DEFINE_THUNK(api_ms_win_crt_conio_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 __conio_common_vcwscanf,
                 uint64_t options,
                 const wchar_t *format,
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

    return i::conio::scan_line_wide(format, arglist);
  }
} // namespace mingw_thunk
