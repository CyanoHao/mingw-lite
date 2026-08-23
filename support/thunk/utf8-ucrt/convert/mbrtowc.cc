#include "../inc/corecrt_internal_mbstring.h"
#include "../inc/corecrt_internal_securecrt.h"

namespace mingw_thunk::ucrt
{

  size_t __cdecl __crt_mbstring::__mbrtowc_utf8(wchar_t *pwc,
                                                const char *s,
                                                size_t n,
                                                mbstate_t *ps,
                                                __crt_cached_ptd_host &ptd)
  {
    static_assert(sizeof(wchar_t) == 2, "wchar_t is assumed to be 16 bits");
    char32_t c32;
    const size_t retval = __mbrtoc32_utf8(&c32, s, n, ps, ptd);
    if (retval <= 4)
    {
      if (c32 > 0xffff)
      {
        c32 = 0xfffd;
      }
      _ASSIGN_IF_NOT_NULL(pwc, static_cast<wchar_t>(c32));
    }
    return retval;
  }

} // namespace mingw_thunk::ucrt
