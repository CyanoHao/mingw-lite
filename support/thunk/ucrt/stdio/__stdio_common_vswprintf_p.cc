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
  // Positional-argument memory variant, wide (plan §M4.2/M4.4,
  // wine-anchored): the narrow-_p rule transposed to wchar units —
  // success needs need_w <= count, the terminator only when
  // need_w < count (need_w == count stores exactly count units with
  // no terminator and still succeeds; need_w > count stores the
  // count-unit prefix and returns -1).
  __DEFINE_THUNK(api_ms_win_crt_stdio_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 __stdio_common_vswprintf_p,
                 uint64_t options,
                 wchar_t *str,
                 size_t count,
                 const wchar_t *format,
                 _locale_t locale,
                 va_list arglist)
  {
    (void)locale;

    if (!str || !format) {
      errno = EINVAL;
      return -1;
    }

    i::shell::wfmt_holder nfmt(format, false, (options & 0x4) != 0);
    if (!nfmt.ok) {
      errno = ENOMEM;
      return -1;
    }

    i::shell::exp_guard guard(options);

    i::shell::wide_render r(nfmt.nfmt, arglist);
    if (!r.ok)
      return -1;

    size_t need_w = i::shell::u8_to_u16(r.buf, r.bytes, nullptr, 0, false);

    if (need_w < count) {
      i::shell::u8_to_u16(r.buf, r.bytes, str, need_w + 1, true);
      return (int)need_w;
    }

    i::shell::u8_to_u16(r.buf, r.bytes, str, count, false);
    return need_w <= count ? (int)need_w : -1;
  }
} // namespace mingw_thunk
