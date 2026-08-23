#include <thunk/_common.h>
#include <thunk/u8crt/musl.h>

#include <errno.h>
#include <stdlib.h>

namespace mingw_thunk
{
  // musl strtox engine (prec 1, frozen-C decimal point).  Native anchors
  // (probe A): endptr stops at the first unconsumed byte; hex floats,
  // inf/infinity/nan(payload) accepted case-insensitively; ERANGE is set
  // on overflow and on inexact underflow but not for an exact denormal;
  // a no-digit prefix returns 0 with errno untouched (the engine's
  // internal EINVAL is masked — native leaves errno alone).  Divergence:
  // null string returns 0 + EINVAL (native narrow crashes; wide native
  // and atof/_wtof natives are graceful 0 + EINVAL — we take the
  // graceful family shape).
  __DEFINE_THUNK(api_ms_win_crt_convert_l1_1_0,
                 0,
                 double,
                 __cdecl,
                 strtod,
                 const char *string,
                 char **end_ptr)
  {
    if (end_ptr)
      *end_ptr = const_cast<char *>(string);
    if (!string) {
      _set_errno(EINVAL);
      return 0.0;
    }

    const int saved_errno = errno;
    double value = musl::strtod(string, end_ptr);
    if (errno == EINVAL)
      errno = saved_errno; // no-digit prefix: not an error natively
    return value;
  }
} // namespace mingw_thunk
