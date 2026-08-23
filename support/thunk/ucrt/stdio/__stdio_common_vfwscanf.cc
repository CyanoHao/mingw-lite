#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/u8crt/musl.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <wchar.h>

#include "wfmt_translate.h"

namespace mingw_thunk
{
  __DEFINE_THUNK(api_ms_win_crt_stdio_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 __stdio_common_vfwscanf,
                 uint64_t options,
                 FILE *stream,
                 const wchar_t *format,
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
      return __ms___stdio_common_vfwscanf(
          options, stream, format, locale, arglist);

    /* the s/c-family rewrites are unconditional on the scanf side:
     * both wide conventions write wchar_t (plan M3.1-3) */
    i::shell::wfmt_holder nfmt(format, true, false);
    if (!nfmt.ok) {
      errno = ENOMEM;
      return -1;
    }

    int fd = _fileno(stream);

    if (musl_ucrt::is_console(fd))
      return musl::vfscanf(musl::g_fp_from_fd(fd), nfmt.nfmt, arglist);

    /* native FILE carrying: the bridge pulls bytes through the same
     * stream and the engine scans them as UTF-8 — wide and narrow
     * reads see the same byte stream (plan M3.1-5) */
    return musl::vfscanf_from_native(stream, nfmt.nfmt, arglist);
  }
} // namespace mingw_thunk
