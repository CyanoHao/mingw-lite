#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/u8crt/musl.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <wchar.h>

#include "printf_shell.h"
#include "wfmt_translate.h"

namespace mingw_thunk
{
  __DEFINE_THUNK(api_ms_win_crt_stdio_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 __stdio_common_vfwprintf,
                 uint64_t options,
                 FILE *stream,
                 const wchar_t *format,
                 _locale_t locale,
                 va_list arglist)
  {
    (void)locale;

    if (!stream || !format) {
      errno = EINVAL;
      return -1;
    }

    /* wide -> narrow format; 0x4 selects the legacy wide conventions
     * (plan M3.1-3: without it the entry is ISO/C99 = musl verbatim) */
    i::shell::wfmt_holder nfmt(format, false, (options & 0x4) != 0);
    if (!nfmt.ok) {
      errno = ENOMEM;
      return -1;
    }

    int fd = _fileno(stream);

    i::shell::exp_guard guard(options);

    if (musl_ucrt::is_console(fd))
      return musl::vfprintf(musl::g_fp_from_fd(fd), nfmt.nfmt, arglist);

    /* native FILE carrying: the engine renders UTF-8 and the bridge
     * writes the bytes back to the same stream — the wide family
     * lands UTF-8 exactly like the narrow family (plan M3.1-5) */
    return musl::vfprintf_to_native(stream, nfmt.nfmt, arglist);
  }
} // namespace mingw_thunk
