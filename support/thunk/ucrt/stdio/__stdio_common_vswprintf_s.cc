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
  // Secure memory variant, wide (plan §M4.2/M4.4, wine-anchored): the
  // M3 narrow-_s rule transposed to wchar units — cap = count + 1
  // stores min(need_w, count) units plus the terminator at [min]
  // (need == count succeeds with the NUL at [count]; count == 0
  // stores only the terminator at [0] and returns -1).
  __DEFINE_THUNK(api_ms_win_crt_stdio_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 __stdio_common_vswprintf_s,
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

    size_t cap = count + 1;
    if (cap == 0) /* count == SIZE_MAX: effectively unbounded */
      cap = count;

    size_t need_w = i::shell::u8_to_u16(r.buf, r.bytes, str, cap, true);
    return need_w <= count ? (int)need_w : -1;
  }
} // namespace mingw_thunk
