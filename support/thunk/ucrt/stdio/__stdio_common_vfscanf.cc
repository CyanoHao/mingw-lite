#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/u8crt/musl.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(api_ms_win_crt_stdio_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 __stdio_common_vfscanf,
                 uint64_t options,
                 FILE *stream,
                 const char *format,
                 _locale_t locale,
                 va_list arglist)
  {
    if (!stream || !format) {
      errno = EINVAL;
      return -1;
    }

    /* _CRT_INTERNAL_SCANF_SECURECRT (0x1): the _s wrappers pass a
     * size_t after every %s/%[/%c pointer, a va_list layout the engine
     * cannot consume — the whole call stays native */
    if (options & 0x1)
      return __ms___stdio_common_vfscanf(
          options, stream, format, locale, arglist);

    int fd = _fileno(stream);

    if (musl_ucrt::is_console(fd))
      return musl::vfscanf(musl::g_fp_from_fd(fd), format, arglist);

    /* native FILE carrying: the bridge pulls bytes through the same
     * stream (locked, at most one lookahead byte pushed back) */
    return musl::vfscanf_from_native(stream, format, arglist);
  }
} // namespace mingw_thunk
