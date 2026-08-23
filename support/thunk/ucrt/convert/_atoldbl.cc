#include <thunk/_common.h>
#include <thunk/u8crt/musl.h>

#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

namespace mingw_thunk
{
  // _LDOUBLE is the raw 10-byte x87 object (mingw stdlib.h); the prec-2
  // engine result is bit-identical in that representation (sizeof(long
  // double) == 12 on i686, first 10 bytes are the x87 value).  rc shape
  // as _atodbl.cc; native r5 crash-on-null is not replicated.
  __DEFINE_THUNK(api_ms_win_crt_convert_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 _atoldbl,
                 _LDOUBLE *result,
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

    memcpy(result->ld, &parsed, 10);
    if (isinf(parsed))
      return 3; // _OVERFLOW
    // target-precision underflow judgment — see _atodbl.cc (the _LDOUBLE
    // target IS the 80-bit parse, so this reduces to a true-zero check
    // plus the engine's total-underflow ERANGE)
    if (parsed == 0.0L && engine_errno == ERANGE)
      return 4; // _UNDERFLOW
    return 0;
  }
} // namespace mingw_thunk
