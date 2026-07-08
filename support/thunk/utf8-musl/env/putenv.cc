#include <thunk/utf8-musl.h>

#include <stdlib.h>
#include <string.h>

#include "../include/stdlib.h"

namespace mingw_thunk
{
  namespace musl
  {
    int __putenv(char *s, size_t l, char *r)
    {
      size_t i = 0;
      if (__environ) {
        for (char **e = __environ; *e; e++, i++)
          if (!_strnicmp(s, *e, l + 1)) {
            char *tmp = *e;
            *e = s;
            __env_rm_add(tmp, r);
            return 0;
          }
      }
      static char **oldenv;
      char **newenv;
      if (__environ == oldenv) {
        newenv = (char **)realloc(oldenv, sizeof *newenv * (i + 2));
        if (!newenv)
          goto oom;
      } else {
        newenv = (char **)malloc(sizeof *newenv * (i + 2));
        if (!newenv)
          goto oom;
        if (i)
          memcpy(newenv, __environ, sizeof *newenv * i);
        free(oldenv);
      }
      newenv[i] = s;
      newenv[i + 1] = 0;
      __environ = oldenv = newenv;
      if (r)
        __env_rm_add(0, r);
      return 0;
    oom:
      free(r);
      return -1;
    }
  } // namespace musl
} // namespace mingw_thunk
