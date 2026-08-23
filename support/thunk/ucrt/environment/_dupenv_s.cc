#include <thunk/_common.h>
#include <thunk/u8crt/musl.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

namespace mingw_thunk
{
  // wine r5/r6: BOTH pointer arguments are validated before anything is
  // written (native leaves *buffer_pointer/*buffer_count untouched on
  // the null-name failure — the reference shell writes first; wine is
  // the oracle).  A miss is rc=0 with ptr=NULL/count=0; a hit mallocs
  // strlen+1 bytes for the caller to free.
  __DEFINE_THUNK(api_ms_win_crt_environment_l1_1_0,
                 0,
                 errno_t,
                 __cdecl,
                 _dupenv_s,
                 char **buffer_pointer,
                 size_t *buffer_count,
                 const char *name)
  {
    if (!buffer_pointer || !name) {
      _set_errno(EINVAL);
      return EINVAL;
    }

    *buffer_pointer = nullptr;
    if (buffer_count)
      *buffer_count = 0;

    const char *value = musl::getenv(name);
    if (!value)
      return 0;

    size_t length = strlen(value) + 1;

    char *duplicate = static_cast<char *>(malloc(length));
    if (!duplicate) {
      _set_errno(ENOMEM);
      return ENOMEM;
    }

    memcpy(duplicate, value, length);
    *buffer_pointer = duplicate;
    if (buffer_count)
      *buffer_count = length;

    return 0;
  }
} // namespace mingw_thunk
