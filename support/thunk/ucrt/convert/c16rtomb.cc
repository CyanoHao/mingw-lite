#include <thunk/_common.h>
#include <thunk/u8crt/mbstring.h>

#include <uchar.h>
#include <wchar.h>

namespace mingw_thunk
{
  // musl adapter behind the shell: a pending high surrogate lives in
  // the caller-given (or internal) mbstate; a lone surrogate returns
  // (size_t)-1 with EILSEQ.  wine's ucrtbase exports this as an
  // unimplemented stub — the overlay replaces it with the real
  // C11/musl state machine (M7 finding, zero-def prediction holds).
  __DEFINE_THUNK(api_ms_win_crt_convert_l1_1_0,
                 0,
                 size_t,
                 __cdecl,
                 c16rtomb,
                 char *s,
                 char16_t c16,
                 mbstate_t *ps)
  {
    return __crt_mbstring::__c16rtomb_utf8(s, c16, ps);
  }
} // namespace mingw_thunk
