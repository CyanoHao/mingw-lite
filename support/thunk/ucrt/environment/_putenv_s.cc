#include <thunk/_common.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

namespace mingw_thunk
{
  // Joins "K=V" and delegates to the overlay _putenv (UTF-8 clean); an
  // empty value deletes the variable (existing _wputenv/musl::unsetenv
  // semantics — wine anchor).  Native EINVAL matrix (r7/r8): null name
  // or null value; documented MSVCRT EINVAL: empty name, '=' in name.
  __DEFINE_THUNK(api_ms_win_crt_environment_l1_1_0,
                 0,
                 errno_t,
                 __cdecl,
                 _putenv_s,
                 const char *name,
                 const char *value)
  {
    if (!name || !*name || strchr(name, '=') || !value) {
      _set_errno(EINVAL);
      return EINVAL;
    }

    size_t name_length = strlen(name);
    size_t value_length = strlen(value);

    char *option =
        static_cast<char *>(malloc(name_length + value_length + 2));
    if (!option) {
      _set_errno(ENOMEM);
      return ENOMEM;
    }

    memcpy(option, name, name_length);
    option[name_length] = '=';
    memcpy(option + name_length + 1, value, value_length + 1);

    errno_t result = _putenv(option) == 0 ? 0 : errno;

    free(option);
    return result;
  }
} // namespace mingw_thunk
