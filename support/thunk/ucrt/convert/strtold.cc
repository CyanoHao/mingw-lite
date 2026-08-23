#include <thunk/_common.h>
#include <thunk/u8crt/musl.h>

#include <errno.h>
#include <stdlib.h>

namespace mingw_thunk
{
  // musl strtox engine, prec 2 — a true 80-bit parse on this target
  // (sizeof(long double) == 12, LDBL_MANT_DIG == 64), matching the native
  // i686 ucrtbase ABI: wine anchors strtold("0.1") bit-identical to the
  // 0.1L literal, "1e310" finite, "1e5000" -> inf + errno 34.  (The
  // mingw import lib cannot even bind this face on i686 — F_LD64 — so
  // the overlay is the first correct binding available.)
  __DEFINE_THUNK(api_ms_win_crt_convert_l1_1_0,
                 0,
                 long double,
                 __cdecl,
                 strtold,
                 const char *string,
                 char **end_ptr)
  {
    if (end_ptr)
      *end_ptr = const_cast<char *>(string);
    if (!string) {
      _set_errno(EINVAL);
      return 0.0L;
    }

    const int saved_errno = errno;
    long double value = musl::strtold(string, end_ptr);
    if (errno == EINVAL)
      errno = saved_errno;
    return value;
  }
} // namespace mingw_thunk
