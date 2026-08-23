#include <thunk/_common.h>
#include "wide_float.h"

#include <errno.h>
#include <stdlib.h>
#include <wchar.h>

namespace mingw_thunk
{
  // See wcstod.cc (D4 view; prec 0 — correctly rounded to float).
  __DEFINE_THUNK(api_ms_win_crt_convert_l1_1_0,
                 0,
                 float,
                 __cdecl,
                 wcstof,
                 const wchar_t *string,
                 wchar_t **end_ptr)
  {
    if (!string) {
      if (end_ptr)
        *end_ptr = nullptr;
      _set_errno(EINVAL);
      return 0.0f;
    }

    const int saved_errno = errno;
    float value =
        u8crt_convert::wide_float_parse<float, musl::strtof>(string, end_ptr);
    if (errno == EINVAL)
      errno = saved_errno;
    return value;
  }
} // namespace mingw_thunk
