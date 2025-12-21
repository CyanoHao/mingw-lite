#include <thunk/_crt.h>
#include <thunk/console.h>
#include <thunk/string.h>
#include <thunk/unicode.h>

#include <stdio.h>

namespace mingw_thunk
{
  __DEFINE_UCRT_THUNK(
      api_ms_win_crt_stdio_l1_1_0, int, putc, int c, FILE *stream)
  {
    return fputc(c, stream);
  }
} // namespace mingw_thunk
