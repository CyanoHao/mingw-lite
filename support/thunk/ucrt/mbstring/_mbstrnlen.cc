#include <thunk/_common.h>

#include "mbs_str.h"

namespace mingw_thunk
{
  // max_count is a *byte* bound here, not a character bound: the return
  // is a character count only when the string ends inside the window,
  // and max_count itself when the bound cuts it.  That is what separates
  // this from _mbsnlen, whose count is characters from the start.  The
  // window is validated before it is counted, so a sequence the bound
  // cuts short is EILSEQ rather than a truncated answer -- which is why
  // this is not _mbstrlen with a smaller max_size.  A null string and a
  // max_count above INT_MAX are EINVAL with (size_t)-1; both checks sit
  // here, in the face, exactly as the reference puts them in
  // _mbstrnlen_l rather than in the body _mbstrlen also calls.
  __DEFINE_THUNK(api_ms_win_crt_multibyte_l1_1_0,
                 0,
                 size_t,
                 __cdecl,
                 _mbstrnlen,
                 const char *string,
                 size_t max_count)
  {
    return mbstring::mbs_strnlen(string, max_count);
  }
} // namespace mingw_thunk
