#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/os.h>
#include <thunk/string.h>
#include <thunk/utf8-musl.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(msvcrt, 0, int, __cdecl, _wputenv, const wchar_t *string)
  {
    // UTF-8
    const wchar_t *eq = wcschr(string, L'=');
    if (!eq) {
      _set_errno(EINVAL);
      return -1;
    }

    d::u_str key;
    if (!key.from_w(string, eq - string)) {
      _set_errno(ENOMEM);
      return -1;
    }

    d::u_str value;
    if (!value.from_w(eq + 1)) {
      _set_errno(ENOMEM);
      return -1;
    }

    int result;
    if (value.size() == 0)
      result = musl::unsetenv(key.c_str(), key.size());
    else
      result =
          musl::setenv(key.c_str(), key.size(), value.c_str(), value.size(), 1);
    if (result != 0)
      return result;

#if THUNK_LEVEL >= NTDDI_WIN4

    __ms__wputenv(string);

#else

    if (i::is_nt()) {
      __ms__wputenv(string);
    } else {
      // Windows 9x: _wputenv cannot update Win32 environment
      d::a_str a_string;
      if (a_string.from_w(string))
        __ms__putenv(a_string.c_str());
    }

#endif

    return 0;
  }
} // namespace mingw_thunk
