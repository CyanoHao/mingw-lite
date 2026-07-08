#include <thunk/utf8-musl.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "../include/stdlib.h"
#include "../include/string.h"

namespace mingw_thunk
{
  namespace musl
  {
    void __env_rm_add(char *old, char *new_)
    {
      static char **env_alloced;
      static size_t env_alloced_n;
      for (size_t i = 0; i < env_alloced_n; i++)
        if (env_alloced[i] == old) {
          env_alloced[i] = new_;
          free(old);
          return;
        } else if (!env_alloced[i] && new_) {
          env_alloced[i] = new_;
          new_ = 0;
        }
      if (!new_)
        return;
      char **t = (char **)realloc(env_alloced, sizeof *t * (env_alloced_n + 1));
      if (!t)
        return;
      (env_alloced = t)[env_alloced_n++] = new_;
    }

    int setenv(
        const char *var, size_t l1, const char *value, size_t l2, int overwrite)
    {
      if (!var || !l1) {
        errno = EINVAL;
        return -1;
      }
      if (!overwrite && getenv(var, l1))
        return 0;

      char *s = (char *)malloc(l1 + l2 + 2);
      if (!s)
        return -1;
      memcpy(s, var, l1);
      s[l1] = '=';
      memcpy(s + l1 + 1, value, l2);
      s[l1 + 1 + l2] = '\0';
      return __putenv(s, l1, s);
    }

    int setenv(const char *var, const char *value, int overwrite)
    {
      size_t l1;
      if (!var || !(l1 = __strchrnul(var, '=') - var) || var[l1]) {
        errno = EINVAL;
        return -1;
      }
      size_t l2 = strlen(value);
      return setenv(var, l1, value, l2, overwrite);
    }
  } // namespace musl
} // namespace mingw_thunk
