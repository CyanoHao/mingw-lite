#include <thunk/_common.h>
#include <thunk/u8crt/musl.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

namespace mingw_thunk
{
  // reference/ucrt/env/getenv.cpp protocol, wine-anchored (probe C +
  // r1–r4): the count slot is written (zeroed) before the buffer
  // validation, the buffer is cleared before the lookup, a miss is
  // rc=0/*len=0 (not an error), a null name is a miss, and the tiny
  // case returns ERANGE WITHOUT setting errno while *len carries the
  // required size (including NUL).
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

    if ((buffer == nullptr) != (buffer_count == 0)) {
      _set_errno(EINVAL);
      return EINVAL;
    }

    if (buffer)
      buffer[0] = 0;

    const char *value = name ? musl::getenv(name) : nullptr;
    if (!value)
      return 0;

    size_t length = strlen(value) + 1;
    *required_count = length;

    if (buffer_count == 0)
      return 0;

    if (length > buffer_count)
      return ERANGE; // errno untouched (wine anchor)

    memcpy(buffer, value, length);
    return 0;
  }
} // namespace mingw_thunk
