#include "../include/string.h"

#include <string.h>

namespace mingw_thunk
{
  namespace musl
  {
    char *__strchrnul(const char *s, int c)
    {
      c = (unsigned char)c;
      if (!c)
        return (char *)s + strlen(s);

      for (; *s && *(unsigned char *)s != c; s++)
        ;
      return (char *)s;
    }
  } // namespace musl
} // namespace mingw_thunk
