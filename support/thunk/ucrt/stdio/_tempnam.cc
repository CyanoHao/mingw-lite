#include <thunk/_common.h>
#include <thunk/string.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(api_ms_win_crt_stdio_l1_1_0,
                 0,
                 char *,
                 __cdecl,
                 _tempnam,
                 const char *dir,
                 const char *prefix)
  {
    d::w_str w_dir;
    if (dir && !w_dir.from_u(dir)) {
      _set_errno(ENOMEM);
      return nullptr;
    }

    d::w_str w_prefix;
    if (prefix && !w_prefix.from_u(prefix)) {
      _set_errno(ENOMEM);
      return nullptr;
    }

    wchar_t *w_result = _wtempnam(dir ? w_dir.c_str() : nullptr,
                                  prefix ? w_prefix.c_str() : nullptr);
    if (!w_result)
      return nullptr;

    d::u_str u_result;
    if (!u_result.from_w(w_result)) {
      free(w_result);
      _set_errno(ENOMEM);
      return nullptr;
    }

    char *result = (char *)malloc(u_result.size() + 1);
    if (result)
      c::strcpy(result, u_result.c_str());
    else
      _set_errno(ENOMEM);

    free(w_result);
    return result;
  }
} // namespace mingw_thunk
