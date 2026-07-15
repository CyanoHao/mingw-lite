#include <thunk/_common.h>
#include <thunk/string.h>

#include <errno.h>
#include <stdlib.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(api_ms_win_crt_environment_l1_1_0,
                 0,
                 errno_t,
                 __cdecl,
                 _putenv_s,
                 const char *name,
                 const char *value)
  {
    if (!name || !value) {
      _set_errno(EINVAL);
      return EINVAL;
    }

    d::w_str wname;
    if (!wname.from_u(name)) {
      _set_errno(ENOMEM);
      return ENOMEM;
    }

    d::w_str wvalue;
    if (!wvalue.from_u(value)) {
      _set_errno(ENOMEM);
      return ENOMEM;
    }

    return _wputenv_s(wname.c_str(), wvalue.c_str());
  }
} // namespace mingw_thunk
