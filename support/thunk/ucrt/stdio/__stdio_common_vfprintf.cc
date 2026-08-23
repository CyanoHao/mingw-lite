#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/u8crt/musl.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>

#include "printf_shell.h"

namespace mingw_thunk
{
  __DEFINE_THUNK(api_ms_win_crt_stdio_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 __stdio_common_vfprintf,
                 uint64_t options,
                 FILE *stream,
                 const char *format,
                 _locale_t locale,
                 va_list arglist)
  {
    (void)locale;

    if (!stream || !format) {
      errno = EINVAL;
      return -1;
    }

    int fd = _fileno(stream);

    i::shell::exp_guard guard(options);

    if (musl_ucrt::is_console(fd))
      return musl::vfprintf(musl::g_fp_from_fd(fd), format, arglist);

    /* native FILE carrying: the engine renders, the bridge writes the
     * bytes back to the same stream (its buffering and position stay
     * authoritative); trailing partial UTF-8 is flushed by the bridge */
    return musl::vfprintf_to_native(stream, format, arglist);
  }
} // namespace mingw_thunk
