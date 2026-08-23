#include <thunk/_common.h>
#include <thunk/string.h>

#include "conio_channel.h"

namespace mingw_thunk
{
  // conio/cputs.cpp: "Writes the given string directly to the console.
  // No newline is appended."  The reference walks the bytes through
  // _putch_nolock, which is what lets a multi-byte character be split
  // across two calls; writing the whole string as one console record
  // cannot split anything, and it is the shape the narrow UTF-8 family
  // wants.  Return value: 0 on success, -1 on failure (the reference's
  // shape, not 1).  wine anchor: -1 under a redirected headless run,
  // with errno left alone.
  __DEFINE_THUNK(api_ms_win_crt_conio_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 _cputs,
                 const char *str)
  {
    if (str == nullptr) {
      // the reference reaches the invalid parameter handler; the
      // graceful shape is the M11 precedent
      errno = EINVAL;
      return -1;
    }

    d::w_str w_str;
    if (!w_str.from_u(str)) {
      errno = ENOMEM;
      return -1;
    }

    if (str[0] == '\0')
      return 0;

    return i::conio::write_wide(w_str.c_str(), w_str.size()) ? 0 : -1;
  }
} // namespace mingw_thunk
