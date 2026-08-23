#include <thunk/_common.h>

#include <stdint.h>
#include <stdio.h>

namespace mingw_thunk
{
  // Positional-argument variant (plan §M4.2): the musl engine detects
  // and handles %N$ natively (nl_arg/nl_type), so the _p entry is pure
  // delegation to the plain shell — the entry name carries the
  // contract, the body is byte-identical routing.
  __DEFINE_THUNK(api_ms_win_crt_stdio_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 __stdio_common_vfprintf_p,
                 uint64_t options,
                 FILE *stream,
                 const char *format,
                 _locale_t locale,
                 va_list arglist)
  {
    return __stdio_common_vfprintf(options, stream, format, locale, arglist);
  }
} // namespace mingw_thunk
