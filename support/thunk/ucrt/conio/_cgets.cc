#include <thunk/_common.h>

#include "conio_channel.h"

#include <stddef.h>

namespace mingw_thunk
{
  // conio/cgets.cpp.  buffer[0] is the caller's maximum string length and
  // buffer[1] is where the length read comes back; the string itself
  // starts at buffer + 2, which is where the reference passes its
  // scratch.  A null buffer or a non-positive length reaches the invalid
  // parameter handler in the reference; the graceful shape is the M11
  // precedent.  Returns buffer + 2 on success and null on failure.
  //
  // wine anchor: null with the string cleared and buffer[1] zeroed under
  // a redirected headless run, errno left alone.
  __DEFINE_THUNK(api_ms_win_crt_conio_l1_1_0,
                 0,
                 char *,
                 __cdecl,
                 _cgets,
                 char *buffer)
  {
    if (buffer == nullptr || buffer[0] <= 0) {
      errno = EINVAL;
      return nullptr;
    }

    size_t count = size_t((unsigned char)buffer[0]);
    size_t size_read = 0;
    errno_t result = i::conio::cgets_s_body(buffer + 2, count, &size_read);
    buffer[1] = (char)size_read;

    return result == 0 ? buffer + 2 : nullptr;
  }
} // namespace mingw_thunk
