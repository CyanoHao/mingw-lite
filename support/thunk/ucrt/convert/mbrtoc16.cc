#include <thunk/_common.h>
#include <thunk/u8crt/mbstring.h>

#include <uchar.h>
#include <wchar.h>

namespace mingw_thunk
{
  // musl adapter behind the shell: non-BMP input splits into a
  // surrogate pair — the second unit is returned as (size_t)-3 on the
  // next call from the pending state.  wine's ucrtbase exports this as
  // an unimplemented stub (M7 finding).
  __DEFINE_THUNK(api_ms_win_crt_convert_l1_1_0,
                 0,
                 size_t,
                 __cdecl,
                 mbrtoc16,
                 char16_t *pc16,
                 const char *s,
                 size_t n,
                 mbstate_t *ps)
  {
    return __crt_mbstring::__mbrtoc16_utf8(pc16, s, n, ps);
  }
} // namespace mingw_thunk
