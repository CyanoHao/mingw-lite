#include <thunk/_common.h>

#include <errno.h>
#include <stdlib.h>
#include <wchar.h>

namespace mingw_thunk
{
  // Wide mirror of _putenv_s: joins L"K=V" and delegates to the overlay
  // _wputenv (which drives the UTF-8 musl environment and mirrors into
  // the native wide environment).  r9 anchor: null value -> EINVAL.
  __DEFINE_THUNK(api_ms_win_crt_environment_l1_1_0,
                 0,
                 errno_t,
                 __cdecl,
                 _wputenv_s,
                 const wchar_t *name,
                 const wchar_t *value)
  {
    if (!name || !*name || wcschr(name, L'=') || !value) {
      _set_errno(EINVAL);
      return EINVAL;
    }

    size_t name_length = wcslen(name);
    size_t value_length = wcslen(value);

    wchar_t *option = static_cast<wchar_t *>(
        malloc((name_length + value_length + 2) * sizeof(wchar_t)));
    if (!option) {
      _set_errno(ENOMEM);
      return ENOMEM;
    }

    wmemcpy(option, name, name_length);
    option[name_length] = L'=';
    wmemcpy(option + name_length + 1, value, value_length + 1);

    errno_t result = _wputenv(option) == 0 ? 0 : errno;

    free(option);
    return result;
  }
} // namespace mingw_thunk
