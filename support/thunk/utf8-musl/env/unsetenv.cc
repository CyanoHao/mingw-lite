#include <errno.h>
#include <string.h>

#include "../include/stdlib.h"
#include "../include/string.h"
#include "../include/unistd.h"

namespace mingw_thunk
{
  namespace musl
  {
    int unsetenv(const char *name, size_t l)
    {
      if (!l) {
        errno = EINVAL;
        return -1;
      }
      if (__environ) {
        char **e = __environ, **eo = e;
        for (; *e; e++)
          if (!_strnicmp(name, *e, l) && l[*e] == '=')
            __env_rm_add(*e, 0);
          else if (eo != e)
            *eo++ = *e;
          else
            eo++;
        if (eo != e)
          *eo = 0;
      }
      return 0;
    }

    int unsetenv(const char *name)
    {
      size_t l = __strchrnul(name, '=') - name;
      if (!l || name[l]) {
        errno = EINVAL;
        return -1;
      }
      return unsetenv(name, l);
    }
  } // namespace musl
} // namespace mingw_thunk
