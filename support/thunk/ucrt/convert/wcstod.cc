#include <thunk/_common.h>
#include "wide_float.h"

#include <errno.h>
#include <stdlib.h>
#include <wchar.h>

namespace mingw_thunk
{
  // D4 view: transcode ws -> UTF-8 (lone surrogates -> U+FFFD), parse
  // with the narrow musl engine, fold the consumed byte count back —
  // every consumable byte is ASCII, so *end = ws + consumed exactly.
  // Native anchors (probe B): L"3.14你" -> 3.14, end at the CJK unit;
  // lone surrogate / CJK prefix -> 0, end at start; null -> 0 + EINVAL
  // (graceful native r2 shape).  Documented divergences: native skips
  // iswspace leading whitespace (U+3000/U+00A0) and stops wide
  // nan-payloads after "nan" — the engine keeps the narrow shapes
  // (plan-3 D13/§3.7).
  __DEFINE_THUNK(api_ms_win_crt_convert_l1_1_0,
                 0,
                 double,
                 __cdecl,
                 wcstod,
                 const wchar_t *string,
                 wchar_t **end_ptr)
  {
    if (!string) {
      if (end_ptr)
        *end_ptr = nullptr;
      _set_errno(EINVAL);
      return 0.0;
    }

    const int saved_errno = errno;
    double value =
        u8crt_convert::wide_float_parse<double, musl::strtod>(string,
                                                              end_ptr);
    if (errno == EINVAL)
      errno = saved_errno; // no-digit prefix: not an error natively
    return value;
  }
} // namespace mingw_thunk
