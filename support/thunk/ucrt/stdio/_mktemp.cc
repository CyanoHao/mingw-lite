#include <thunk/_common.h>

#include <errno.h>
#include <io.h>
#include <string.h>

namespace mingw_thunk
{
  // reference/ucrt/lowio/mktemp.cpp shape, UTF-8 edition: the last five
  // trailing X's take the thread-id decimal digits, one more X takes a
  // probe letter a..z (existence probed via this layer's _access
  // thunk, so CJK prefixes keep UTF-8 semantics).  The DBCS
  // trail-byte guard of the reference is unnecessary here: 'X' can
  // never be a UTF-8 continuation byte (self-synchronizing encoding).
  __DEFINE_THUNK(api_ms_win_crt_stdio_l1_1_0,
                 0,
                 char *,
                 __cdecl,
                 _mktemp,
                 char *template_string)
  {
    if (!template_string) {
      _set_errno(EINVAL);
      return nullptr;
    }

    size_t length = strlen(template_string);
    return _mktemp_s(template_string, length + 1) == 0 ? template_string
                                                       : nullptr;
  }
} // namespace mingw_thunk
