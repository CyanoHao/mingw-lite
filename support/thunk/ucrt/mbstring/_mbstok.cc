#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // The static-context twin of _mbstok_s.  The reference keeps one
  // token pointer per thread; the same storage is used here so the two
  // faces cannot drift apart, which is the whole point of the reference
  // commenting on why it calls the non-secure entry point.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 unsigned char *,
                 __cdecl,
                 _mbstok,
                 unsigned char *string,
                 const unsigned char *control)
  {
    static unsigned char *context = nullptr;
    return mbstring::tok_split(string, control, &context);
  }
} // namespace mingw_thunk
