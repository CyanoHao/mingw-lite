#include <thunk/_common.h>
#include <thunk/u8crt/mbstring.h>

#include <wchar.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(api_ms_win_crt_string_l1_1_0,
                 0,
                 size_t,
                 __cdecl,
                 mbrlen,
                 const char *s,
                 size_t n,
                 mbstate_t *ps)
  {
    static mbstate_t mbst = {};

    return __crt_mbstring::__mbrtoc32_utf8(nullptr, s, n, ps ? ps : &mbst);
  }
} // namespace mingw_thunk
