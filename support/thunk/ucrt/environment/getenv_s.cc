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
                 getenv_s,
                 size_t *required_count,
                 char *buffer,
                 size_t buffer_count,
                 const char *name)
  {
    if (!required_count) {
      _set_errno(EINVAL);
      return EINVAL;
    }
    *required_count = 0;

    if ((buffer && buffer_count == 0) || (!buffer && buffer_count != 0)) {
      _set_errno(EINVAL);
      return EINVAL;
    }

    if (buffer)
      buffer[0] = 0;

    if (!name)
      return 0;

    char *value = musl::getenv(name);
    if (!value)
      return 0;

    size_t const len = strlen(value);
    *required_count = len + 1;

    if (buffer_count == 0)
      return 0;

    if (len + 1 > buffer_count)
      return ERANGE;

    memcpy(buffer, value, len);
    buffer[len] = 0;
    return 0;
  }
} // namespace mingw_thunk
