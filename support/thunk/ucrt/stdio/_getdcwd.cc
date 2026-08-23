#include <thunk/_common.h>
#include <thunk/string.h>

#include <direct.h>
#include <errno.h>

namespace mingw_thunk
{
  // Drive-qualified _getcwd (plan §M5.1-2): the wide side has no
  // conversion concerns, so _wgetdcwd stays native (plain dllimport,
  // the _getcwd -> _wgetcwd mechanism); the narrow answer is decoded
  // from the wide path.  Drive 0 means the current drive; the drive
  // number is passed through unvalidated (native behavior).
  __DEFINE_THUNK(api_ms_win_crt_stdio_l1_1_0,
                 0,
                 char *,
                 __cdecl,
                 _getdcwd,
                 int drive,
                 char *buf,
                 int size)
  {
    wchar_t *w_res = _wgetdcwd(drive, nullptr, 0);

    if (w_res == nullptr)
      return nullptr;

    d::u_str res;
    if (!res.from_w(w_res)) {
      free(w_res);
      _set_errno(ENOMEM);
      return nullptr;
    }
    free(w_res);

    if (buf && res.size() >= size) {
      _set_errno(ERANGE);
      return nullptr;
    }

    if (!buf) {
      if (res.size() >= size)
        size = res.size() + 1;
      buf = (char *)malloc(size);
    }

    memcpy(buf, res.c_str(), res.size());
    buf[res.size()] = 0;
    return buf;
  }
} // namespace mingw_thunk
