#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // Delimiter matching is by whole character on both sides, and a token
  // always begins and ends on a character boundary: the delimiter's first
  // byte is blanked and the context resumes one whole character later.
  // A null context or charset, or a null string with an empty context, is
  // EINVAL -- the reference's three validation returns.
  // wine anchors: the SBCS token round trip, tok(all-separators) == null,
  // and both invalid-argument shapes answer null.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 unsigned char *,
                 __cdecl,
                 _mbstok_s,
                 unsigned char *string,
                 const unsigned char *control,
                 unsigned char **context)
  {
    return mbstring::tok_split(string, control, context);
  }
} // namespace mingw_thunk
