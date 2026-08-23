#include <thunk/_common.h>
#include <thunk/u8crt/mbstring.h>

#include <string.h>
#include <wchar.h>

namespace mingw_thunk
{
  // UCRT __crt_mbstring::_wcrtomb_internal
  __DEFINE_THUNK(api_ms_win_crt_convert_l1_1_0,
                 0,
                 size_t,
                 __cdecl,
                 wcrtomb,
                 char *s,
                 wchar_t wc,
                 mbstate_t *ps)
  {
    using namespace __crt_mbstring;

    static mbstate_t local_state{};
    return __c32rtomb_utf8(
        s, static_cast<char32_t>(wc), ps ? ps : &local_state);
  }
} // namespace mingw_thunk
