#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/string.h>
#include <thunk/utf8-musl.h>

#include <errno.h>
#include <stdlib.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(api_ms_win_crt_environment_l1_1_0,
                 0,
                 errno_t,
                 __cdecl,
                 _wputenv_s,
                 const wchar_t *name,
                 const wchar_t *value)
  {
    if (!name || !value) {
      _set_errno(EINVAL);
      return EINVAL;
    }

    d::u_str key;
    if (!key.from_w(name)) {
      _set_errno(ENOMEM);
      return ENOMEM;
    }

    int result;
    if (value[0] == 0)
      result = musl::unsetenv(key.c_str(), key.size());
    else {
      d::u_str val;
      if (!val.from_w(value)) {
        _set_errno(ENOMEM);
        return ENOMEM;
      }
      result =
          musl::setenv(key.c_str(), key.size(), val.c_str(), val.size(), 1);
    }

    if (result != 0)
      return errno;

    __ms__wputenv_s(name, value);

    return 0;
  }
} // namespace mingw_thunk
