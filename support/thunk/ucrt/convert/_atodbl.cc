#include <thunk/_common.h>
#include <thunk/u8crt/musl.h>

#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

namespace mingw_thunk
{
  // ucrt corecrt_math.h: _OVERFLOW 3 / _UNDERFLOW 4 (return codes of the
  // _atoxx family; not defined by the mingw headers).
  enum : int
  {
    u8crt_overflow_rc = 3,
    u8crt_underflow_rc = 4
  };

  // Shared shape (reference atof.cpp + wine probe D): parse through the
  // 80-bit engine (native parses into an _LDBL12 intermediate and narrows
  // — the same rounding chain), report _OVERFLOW for infinities (a parsed
  // "inf" counts too: native _atodbl("inf") -> 3), _UNDERFLOW for a total
  // underflow of the target type (engine ERANGE + zero result), 0
  // otherwise — including "abc"/""/trailing garbage (rc 0, value 0).
  // errno is left untouched (native never sets it here; the engine's
  // ERANGE/EINVAL are masked).  Null pointers are graceful EINVAL +
  // errno EINVAL (native invokes the invalid-parameter handler and dies
  // — r3/r15; we do not replicate the crash).
  __DEFINE_THUNK(api_ms_win_crt_convert_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 _atodbl,
                 _CRT_DOUBLE *result,
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

    result->x = (double)parsed;
    if (isinf(result->x))
      return u8crt_overflow_rc;
    // Underflow is judged at the TARGET precision (native parses the
    // status at the target type: 1e-999 is a normal 80-bit but zeroes
    // the double -> _UNDERFLOW; "abc"/"0" parse to a true zero -> 0)
    if (result->x == 0.0 && (parsed != 0.0L || engine_errno == ERANGE))
      return u8crt_underflow_rc;
    return 0;
  }
} // namespace mingw_thunk
