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
                 __stdio_common_vsnwprintf_s,
                 uint64_t options,
                 wchar_t *str,
                 size_t size,
                 size_t maxcount,
                 const wchar_t *format,
                 _locale_t locale,
                 va_list arglist)
  {
    (void)locale;

    if (!format) {
      errno = EINVAL;
      return -1;
    }

    i::shell::wfmt_holder nfmt(format, false, (options & 0x4) != 0);
    if (!nfmt.ok) {
      errno = ENOMEM;
      return -1;
    }

    /* _TRUNCATE ((size_t)-1) or a roomy maxcount bound the render by
     * the buffer; a tighter maxcount bounds it by maxcount+1 (wide
     * units: maxcount characters + terminator).  Truncation returns
     * -1 with the terminator guaranteed. */
    size_t cap =
        (maxcount == (size_t)-1 || maxcount >= size) ? size : maxcount + 1;

    if (cap == 0 || !str) {
      errno = EINVAL;
      return -1;
    }

    i::shell::exp_guard guard(options);

    i::shell::wide_render r(nfmt.nfmt, arglist);
    if (!r.ok)
      return -1;

    size_t need_w = i::shell::u8_to_u16(r.buf, r.bytes, nullptr, 0, false);
    i::shell::u8_to_u16(r.buf, r.bytes, str, cap, true);
    return need_w < cap ? (int)need_w : -1;
  }
} // namespace mingw_thunk
