#include <thunk/_common.h>
#include <thunk/u8crt/musl.h>

#include <errno.h>
#include <math.h>
#include <stdlib.h>

namespace mingw_thunk
{
  // See _atodbl.cc (80-bit engine, narrowed to float; rc shape identical).
  __DEFINE_THUNK(api_ms_win_crt_convert_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 _atoflt,
                 _CRT_FLOAT *result,
                 char *string)
  {
    if (!result || !string) {
      _set_errno(EINVAL);
      return EINVAL;
    }

    const int saved_errno = errno;
    errno = 0;
    const long double parsed = musl::strtold(string, nullptr);
    const int engine_errno = errno;
    errno = saved_errno;

    result->f = (float)parsed;
    if (isinf(result->f))
      return 3; // _OVERFLOW
    // target-precision underflow judgment — see _atodbl.cc
    if (result->f == 0.0f && (parsed != 0.0L || engine_errno == ERANGE))
      return 4; // _UNDERFLOW
    return 0;
  }
} // namespace mingw_thunk
