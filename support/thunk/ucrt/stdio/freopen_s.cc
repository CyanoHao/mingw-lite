#include <thunk/_common.h>

#include <errno.h>
#include <stdio.h>

namespace mingw_thunk
{
  // Secure reopen: validate, then forward to this layer's own freopen
  // thunk (UTF-8 -> UTF-16 path transcode + console_channel_on_open
  // fully reused).  wine anchor (M7): validation failures leave
  // *pFile UNTOUCHED (unlike fopen_s, which pre-nulls) and return
  // EINVAL without raising the handler; a failed reopen nulls *pFile
  // and returns the errno code (fopen_s M5 shape).
  __DEFINE_THUNK(api_ms_win_crt_stdio_l1_1_0,
                 0,
                 errno_t,
                 __cdecl,
                 freopen_s,
                 FILE **pFile,
                 const char *path,
                 const char *mode,
                 FILE *stream)
  {
    if (!pFile) {
      _set_errno(EINVAL);
      return EINVAL;
    }

    if (!path || !mode || !stream) {
      _set_errno(EINVAL);
      return EINVAL;
    }

    FILE *fp = freopen(path, mode, stream);
    if (!fp) {
      int e = EINVAL;
      _get_errno(&e);
      *pFile = nullptr;
      return e;
    }

    *pFile = fp;
    return 0;
  }
} // namespace mingw_thunk
