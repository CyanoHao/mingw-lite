#include <string.h>

#include "../include/string.h"
#include "../include/unistd.h"

namespace mingw_thunk
{
  namespace musl
  {
    char *getenv(const char *name, size_t l)
    {
      if (l && __environ)
        for (char **e = __environ; *e; e++)
          if (!_strnicmp(name, *e, l) && l[*e] == '=')
            return *e + l + 1;
      return 0;
    }

    char *getenv(const char *name)
    {
      size_t l = __strchrnul(name, '=') - name;
      if (l && !name[l])
        return getenv(name, l);
      return 0;
    }
  } // namespace musl
} // namespace mingw_thunk
