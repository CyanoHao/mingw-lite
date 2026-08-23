#include <thunk/_common.h>
#include <thunk/u8crt/mbstring.h>

#include <uchar.h>
#include <wchar.h>

namespace mingw_thunk
{
  // musl adapter behind the shell: full code point out, EILSEQ for
  // surrogate-encoded (CESU) input.  wine's ucrtbase exports this as
  // an unimplemented stub (M7 finding).
  __DEFINE_THUNK(api_ms_win_crt_convert_l1_1_0,
                 0,
                 size_t,
                 __cdecl,
                 c32rtomb,
                 char *s,
                 char32_t c32,
                 mbstate_t *ps)
  {
    return __crt_mbstring::__c32rtomb_utf8(s, c32, ps);
  }
} // namespace mingw_thunk
