#include <thunk/_common.h>
#include <thunk/string.h>

#include <errno.h>
#include <stdio.h>
#include <wchar.h>

namespace mingw_thunk
{
  extern "C" errno_t __cdecl _wtmpnam_s(wchar_t *dst, size_t size);

  // tmpnam shell + _s protocol, wine-anchored: null buffer -> EINVAL;
  // zero size (or a buffer too small for the name) -> ERANGE with
  // buffer[0] reset; success -> 0 with the native wide name transcoded
  // to UTF-8 (names are pure ASCII, so the byte count matches).
  __DEFINE_THUNK(
      api_ms_win_crt_stdio_l1_1_0, 0, errno_t, __cdecl, tmpnam_s, char *buffer, size_t size)
  {
    if (!buffer) {
      _set_errno(EINVAL);
      return EINVAL;
    }

    if (size == 0) {
      _set_errno(ERANGE);
      return ERANGE;
    }

    wchar_t w_buffer[L_tmpnam];
    errno_t rc = _wtmpnam_s(w_buffer, L_tmpnam);
    if (rc != 0) {
      buffer[0] = 0;
      _set_errno(rc);
      return rc;
    }

    int needed = d::u_str::size_from_w(w_buffer, -1);
    if (needed < 0 || size_t(needed) > size) {
      buffer[0] = 0;
      _set_errno(ERANGE);
      return ERANGE;
    }

    if (!d::u_str::best_effort_from_w(buffer, (int)size, w_buffer)) {
      buffer[0] = 0;
      _set_errno(ERANGE);
      return ERANGE;
    }
    return 0;
  }
} // namespace mingw_thunk
