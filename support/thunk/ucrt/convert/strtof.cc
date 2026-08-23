#include <thunk/_common.h>
#include <thunk/u8crt/musl.h>

#include <errno.h>
#include <stdlib.h>

namespace mingw_thunk
{
  // musl strtox engine, prec 0 — correctly rounded to float (native
  // anchor: strtof("1.0000000596046448") = 1.00000012; strtof("0.1")
  // bit-exact).  ERANGE shapes as strtod; see strtod.cc.
  __DEFINE_THUNK(api_ms_win_crt_convert_l1_1_0,
                 0,
                 float,
                 __cdecl,
                 strtof,
                 const char *string,
                 char **end_ptr)
  {
    if (end_ptr)
      *end_ptr = const_cast<char *>(string);
    if (!string) {
      _set_errno(EINVAL);
      return 0.0f;
    }

    const int saved_errno = errno;
    float value = musl::strtof(string, end_ptr);
    if (errno == EINVAL)
      errno = saved_errno;
    return value;
  }
} // namespace mingw_thunk
