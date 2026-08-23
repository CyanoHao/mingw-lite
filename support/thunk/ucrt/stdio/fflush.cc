#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/u8crt/musl.h>

#include <stdio.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(
      api_ms_win_crt_stdio_l1_1_0, 0, int, __cdecl, fflush, FILE *stream)
  {
    if (!stream) {
      /* flush every occupied console channel (g_stdout, g_stderr and
       * the fd >= 3 slot pool) plus the native streams */
      musl::console_channel_flush_all();
      return __ms_fflush(nullptr);
    }

    int fd = _fileno(stream);
    if (!musl_ucrt::is_console(fd))
      return __ms_fflush(stream);

    return musl::fflush(musl::g_fp_from_fd(fd));
  }
} // namespace mingw_thunk
