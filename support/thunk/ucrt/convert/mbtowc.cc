#include <thunk/_common.h>
#include <thunk/u8crt/mbstring.h>

#include <stdlib.h>

namespace mingw_thunk
{
  // UCRT __crt_mbstring::_mbtowc_internal
  __DEFINE_THUNK(api_ms_win_crt_convert_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 mbtowc,
                 wchar_t *pwc,
                 const char *s,
                 size_t n)
  {
    static mbstate_t internal_state{};
    if (!s || n == 0) {
      /* indicate do not have state-dependent encodings,
      handle zero length string */
      internal_state = {};
      return 0;
    }

    if (!*s) {
      /* handle NULL char */
      if (pwc) {
        *pwc = 0;
      }
      return 0;
    }

    int result = static_cast<int>(mbrtowc(pwc, s, n, &internal_state));
    if (result < 0)
      result = -1;
    return result;
  }
} // namespace mingw_thunk
