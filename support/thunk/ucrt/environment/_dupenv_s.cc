#include <thunk/_common.h>
#include <thunk/utf8-musl.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(api_ms_win_crt_environment_l1_1_0,
                 0,
                 errno_t,
                 __cdecl,
                 _dupenv_s,
                 char **buffer,
                 size_t *numberOfElements,
                 const char *varname)
  {
    if (!buffer || !varname) {
      _set_errno(EINVAL);
      return EINVAL;
    }

    *buffer = nullptr;
    if (numberOfElements)
      *numberOfElements = 0;

    char *value = musl::getenv(varname);
    if (!value) {
      return 0;
    }

    size_t len = strlen(value) + 1;
    *buffer = (char *)malloc(len);
    if (!*buffer) {
      _set_errno(ENOMEM);
      return ENOMEM;
    }

    memcpy(*buffer, value, len);
    if (numberOfElements)
      *numberOfElements = len;

    return 0;
  }
} // namespace mingw_thunk
