#include <thunk/_common.h>

#include <io.h>

namespace mingw_thunk
{
  // Internal _sopen-family dispatcher (exported without the w prefix —
  // api-set §3.6.2 note).  The variadic tail is a legacy security
  // cookie slot that the native implementation ignores; it is left
  // unread (cdecl callers pass 5 or 6 arguments).  Pure delegation to
  // this layer's own _sopen_s thunk.
  __DEFINE_THUNK(api_ms_win_crt_stdio_l1_1_0,
                 0,
                 errno_t,
                 __cdecl,
                 _sopen_dispatch,
                 const char *path,
                 int oflag,
                 int shflag,
                 int pmode,
                 int *fd,
                 ...)
  {
    return _sopen_s(fd, path, oflag, shflag, pmode);
  }
} // namespace mingw_thunk
