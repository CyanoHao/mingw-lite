#include "__p__environ.h"

#include <thunk/_common.h>
#include <thunk/string.h>

#include <stdlib.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(crtdll, 0, char ***, __cdecl, __p__environ)
  {
    while (!__atomic_test_and_set(&i::u8_environ_lock, __ATOMIC_ACQUIRE))
      Sleep(0);

    if (i::u8_environ == nullptr) {
      i::u8_environ = (char **)malloc(sizeof(char *));
      i::u8_environ[0] = nullptr;
    };

    __atomic_clear(&i::u8_environ_lock, __ATOMIC_RELEASE);
    return &i::u8_environ;
  }

  namespace i
  {
    char **u8_dup_envp(char **envp, size_t size)
    {
      char **new_environ = (char **)malloc(sizeof(char *) * (size + 1));
      for (size_t i = 0; i < size; ++i)
        new_environ[i] = _strdup(envp[i]);
      new_environ[size] = nullptr;
      return new_environ;
    }

    char **u8_envp = nullptr;

    char **u8_environ = nullptr;
    size_t u8_environ_size = 0;
    bool u8_environ_lock = false;
  } // namespace i
} // namespace mingw_thunk
