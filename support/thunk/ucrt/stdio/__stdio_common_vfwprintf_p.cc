#include <thunk/_common.h>

#include <stdint.h>
#include <stdio.h>
#include <wchar.h>

namespace mingw_thunk
{
  // Positional-argument variant, wide (plan §M4.2): same delegation
  // as the narrow _p — the pre-translator keeps %N$ prefixes intact
  // and the engine resolves them.
  __DEFINE_THUNK(api_ms_win_crt_stdio_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 __stdio_common_vfwprintf_p,
                 uint64_t options,
                 FILE *stream,
                 const wchar_t *format,
                 _locale_t locale,
                 va_list arglist)
  {
    return __stdio_common_vfwprintf(options, stream, format, locale, arglist);
  }
} // namespace mingw_thunk
