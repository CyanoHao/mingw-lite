#include <thunk/_common.h>
#include "wide_float.h"

#include <errno.h>
#include <stdlib.h>
#include <wchar.h>

namespace mingw_thunk
{
  // _wtof == wcstod without endptr (reference atof.cpp shape); null ->
  // 0 + errno EINVAL (native r12 graceful anchor).
  __DEFINE_THUNK(api_ms_win_crt_convert_l1_1_0,
                 0,
                 double,
                 __cdecl,
                 _wtof,
                 const wchar_t *string)
  {
    if (!string) {
      _set_errno(EINVAL);
      return 0.0;
    }

    const int saved_errno = errno;
    double value =
        u8crt_convert::wide_float_parse<double, musl::strtod>(string, nullptr);
    if (errno == EINVAL)
      errno = saved_errno;
    return value;
  }
} // namespace mingw_thunk
