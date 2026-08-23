#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/u8crt/ucrt.h>

#include <stdio.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(
      api_ms_win_crt_stdio_l1_1_0, 0, int, __cdecl, fclose, FILE *stream)
  {
    if (!stream) {
      return 0;
    }

    ucrt::console_close(_fileno(stream));
    return __ms_fclose(stream);
  }
} // namespace mingw_thunk
