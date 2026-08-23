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
                 __stdio_common_vswprintf,
                 uint64_t options,
                 wchar_t *str,
                 size_t len,
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

    i::shell::exp_guard guard(options);

    /* _vscwprintf-style measurement (str == NULL, len == 0): the
     * answer counts wchar units, so the render still has to happen */
    if (len == 0) {
      i::shell::wide_render r(nfmt.nfmt, arglist);
      if (!r.ok)
        return -1;
      return (int)i::shell::u8_to_u16(r.buf, r.bytes, nullptr, 0, false);
    }

    if (!str) {
      errno = EINVAL;
      return -1;
    }

    /* the engine renders UTF-8; every mode funnels through one full
     * scratch render and then transcodes into the wide buffer */
    i::shell::wide_render r(nfmt.nfmt, arglist);
    if (!r.ok)
      return -1;

    size_t need_w = i::shell::u8_to_u16(r.buf, r.bytes, nullptr, 0, false);

    /* C11 swprintf (options 0x2): terminator always written, the
     * required wide length survives truncation */
    if (options & 0x2) {
      i::shell::u8_to_u16(r.buf, r.bytes, str, len, true);
      return (int)need_w;
    }

    /* legacy unbounded (bit 0x1, len == (size_t)-1) */
    if (len == (size_t)-1) {
      i::shell::u8_to_u16(r.buf, r.bytes, str, (size_t)-1, true);
      return (int)need_w;
    }

    /* legacy _snwprintf (bit 0x1, bounded): fits -> data plus
     * terminator and the length; truncated -> exactly len wide units
     * with NO terminator and -1 (the unit past the count stays
     * untouched) */
    if (need_w < len) {
      i::shell::u8_to_u16(r.buf, r.bytes, str, len, true);
      return (int)need_w;
    }
    i::shell::u8_to_u16(r.buf, r.bytes, str, len, false);
    return -1;
  }
} // namespace mingw_thunk
