#include <thunk/_common.h>
#include <thunk/u8crt/mbstring.h>

#include <wchar.h>

namespace mingw_thunk
{
  // UCRT __crt_mbstring::__mbrtowc_utf8
  __DEFINE_THUNK(api_ms_win_crt_convert_l1_1_0,
                 0,
                 size_t,
                 __cdecl,
                 mbrtowc,
                 wchar_t *pwc,
                 const char *s,
                 size_t n,
                 mbstate_t *ps)
  {
    using namespace __crt_mbstring;

    char32_t c32;
    const size_t retval = __mbrtoc32_utf8(&c32, s, n, ps);
    // If we succesfully consumed a character, write the result after a quick
    // range check
    if (retval <= 4) {
      if (c32 > 0xffff) {
        // A 4-byte UTF-8 character won't fit into a single UTF-16 wchar
        // So return the "replacement char"
        c32 = 0xfffd;
      }
      if (pwc)
        *pwc = static_cast<wchar_t>(c32);
    }
    return retval;
  }
} // namespace mingw_thunk
