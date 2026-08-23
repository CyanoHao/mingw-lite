#include <thunk/_common.h>
#include <thunk/u8crt/mbstring.h>

#include <stdlib.h>
#include <wchar.h>

namespace mingw_thunk
{
  // wctomb == wcrtomb with a private static state, folded to int
  // (plan §M5.1-3): the engine writes UTF-8 for every code point
  // (L'\0' stores one NUL byte); EILSEQ paths return (size_t)-1 and
  // fold to -1.  Native walks the ACP by locale; ours is UTF-8
  // unconditionally (accepted divergence, plan M5.4-1).
  __DEFINE_THUNK(api_ms_win_crt_convert_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 wctomb,
                 char *s,
                 wchar_t wc)
  {
    using namespace __crt_mbstring;

    static mbstate_t local_state{};
    size_t r = __c32rtomb_utf8(s, static_cast<char32_t>(wc), &local_state);
    return r == (size_t)-1 ? -1 : (int)r;
  }
} // namespace mingw_thunk
