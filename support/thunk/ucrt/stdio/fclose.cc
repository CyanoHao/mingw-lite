#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/u8crt/musl.h>

#include <stdio.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(
      api_ms_win_crt_stdio_l1_1_0, 0, int, __cdecl, fclose, FILE *stream)
  {
    if (stream) {
      /* release (and flush) the console channel while the fd is still
       * open; fd 0/1/2 only flush the static trio */
      musl::console_channel_release(_fileno(stream));
    }
    return __ms_fclose(stream);
  }
} // namespace mingw_thunk
