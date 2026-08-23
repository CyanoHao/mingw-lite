#include <thunk/_common.h>

#include "conio_channel.h"

#include <stddef.h>

namespace mingw_thunk
{
  // The whole face is the shared protocol: _cgets is _cgets_s with the
  // length handshake in front of it, so there is one body in
  // conio_channel.h and this entry only marshals the caller's
  // buffer[0]/buffer[1] window.
  __DEFINE_THUNK(api_ms_win_crt_conio_l1_1_0,
                 0,
                 errno_t,
                 __cdecl,
                 _cgets_s,
                 char *buffer,
                 size_t count,
                 size_t *size_read)
  {
    return i::conio::cgets_s_body(buffer, count, size_read);
  }
} // namespace mingw_thunk
