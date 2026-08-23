#include <thunk/_common.h>
#include <thunk/u8crt/ucrt.h>

#include <stdint.h>
#include <stdio.h>
#include <wchar.h>

namespace mingw_thunk
{
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
    return ucrt::__stdio_common_vfwprintf_p(
        options, stream, format, locale, arglist);
  }
} // namespace mingw_thunk
