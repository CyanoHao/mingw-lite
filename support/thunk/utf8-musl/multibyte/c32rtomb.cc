#include "../include/uchar.h"
#include "../include/wchar.h"

#include <errno.h>

namespace mingw_thunk
{
  namespace musl
  {
    size_t c32rtomb(char *s, char32_t c32, mbstate_t *ps)
    {
      return wcrtomb(s, c32, ps);
    }
  } // namespace musl
} // namespace mingw_thunk
