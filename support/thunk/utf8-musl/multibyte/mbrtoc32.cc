#include "../include/uchar.h"
#include "../include/wchar.h"

namespace mingw_thunk
{
  namespace musl
  {
    size_t mbrtoc32(char32_t *pc32, const char *s, size_t n, mbstate_t *ps)
    {
      static unsigned internal_state;
      if (!ps)
        ps = (mbstate_t *)&internal_state;
      if (!s)
        return mbrtoc32(0, "", 1, ps);
      char32_t wc;
      size_t ret = mbrtowc(&wc, s, n, ps);
      if (ret <= 4 && pc32)
        *pc32 = wc;
      return ret;
    }
  } // namespace musl
} // namespace mingw_thunk
