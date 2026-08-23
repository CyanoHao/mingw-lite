#include "../inc/corecrt_internal_mbstring.h"

#include <locale.h>
#include <stdlib.h>
#include <wchar.h>

namespace mingw_thunk::ucrt
{

  using namespace __crt_mbstring;

  int __cdecl _mbtowc_internal(wchar_t *pwc,
                               const char *s,
                               size_t n,
                               __crt_cached_ptd_host &ptd)
  {
    static mbstate_t internal_state{};
    if (!s || n == 0)
    {
      internal_state = {};
      return 0;
    }

    if (!*s)
    {
      if (pwc)
      {
        *pwc = 0;
      }
      return 0;
    }

    int result =
        static_cast<int>(__mbrtowc_utf8(pwc, s, n, &internal_state, ptd));
    if (result < 0)
      result = -1;
    return result;
  }

} // namespace mingw_thunk::ucrt
