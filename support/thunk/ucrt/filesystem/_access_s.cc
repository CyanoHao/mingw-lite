#include <thunk/_common.h>
#include <thunk/string.h>

#include <errno.h>
#include <io.h>
#include <stdlib.h>

namespace mingw_thunk
{
  // reference/ucrt/filesystem/access.cpp shape: null path and mode
  // validation are the native _waccess_s's job (probe D: mode 7/-1 ->
  // EINVAL; r15: null -> EINVAL; invalid UTF-8 bytes -> the probe goes
  // through and misses -> ENOENT, wine-anchored shape).
  __DEFINE_THUNK(api_ms_win_crt_filesystem_l1_1_0,
                 0,
                 errno_t,
                 __cdecl,
                 _access_s,
                 const char *path,
                 int access_mode)
  {
    if (!path)
      return _waccess_s(nullptr, access_mode);

    d::w_str wide_path;
    if (!wide_path.from_u(path)) {
      // Invalid UTF-8: fall back to the longest valid prefix.  The
      // resulting name is never the file the caller meant (a miss
      // either way), which reproduces the native unconvertible-miss
      // shape without a false positive.
      size_t capacity = strlen(path) + 1;
      wchar_t *buffer =
          static_cast<wchar_t *>(malloc(capacity * sizeof(wchar_t)));
      if (!buffer) {
        _set_errno(ENOMEM);
        return ENOMEM;
      }

      errno_t result = EINVAL;
      if (d::w_str::best_effort_from_u(buffer, int(capacity), path))
        result = _waccess_s(buffer, access_mode);

      free(buffer);
      return result;
    }

    return _waccess_s(wide_path.c_str(), access_mode);
  }
} // namespace mingw_thunk
