#include <thunk/_common.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

namespace mingw_thunk
{
  // Secure fopen (plan §M5.1-1, wine-anchored): validate, null the
  // out-pointer up front, then forward to this layer's own fopen thunk
  // (UTF-8 -> UTF-16 path transcode + console_channel_on_open fully
  // reused).  On failure the return IS the errno code (missing file ->
  // ENOENT) and *pFile stays NULL; the invalid-parameter handler is
  // never raised (wine default parity).  msvcrt.dll has no fopen_s
  // export — ucrt-only (plan M5.1-1).
  __DEFINE_THUNK(api_ms_win_crt_stdio_l1_1_0,
                 0,
                 errno_t,
                 __cdecl,
                 fopen_s,
                 FILE **pFile,
                 const char *filename,
                 const char *mode)
  {
    if (!pFile) {
      _set_errno(EINVAL);
      return EINVAL;
    }

    *pFile = nullptr;

    if (!filename || !mode) {
      _set_errno(EINVAL);
      return EINVAL;
    }

    FILE *fp = fopen(filename, mode);
    if (!fp) {
      int e = EINVAL;
      _get_errno(&e);
      return e;
    }

    *pFile = fp;
    return 0;
  }
} // namespace mingw_thunk
