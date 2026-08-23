#include <thunk/_common.h>
#include <thunk/string.h>
#include <thunk/u8crt/musl.h>

#include <corecrt_startup.h>
#include <stdlib.h>

#include <unistd.h>
#include <windows.h>

namespace mingw_thunk
{
  namespace i
  {
    char **u8envp_from_wenvp(wchar_t **wenvp);
    char **frozen_envp_copy(char **envp);

    // Definition lives in the regular archive set (this file): the
    // startup-dep member that assigns it is only linked in some
    // configurations.
    char **g_initial_u8envp = nullptr;
  } // namespace i

  // The initial environment is a frozen deep copy of the startup-time
  // UTF-8 block (wine anchor: it stays pristine after _putenv, while
  // the live musl::__environ array is replaced/mutated).  If the
  // startup path resolved to the native initializer (link orders that
  // never pulled our startup-dep member — e.g. the test binaries),
  // this lazily bootstraps the UTF-8 environment before snapshotting.
  __DEFINE_THUNK(api_ms_win_crt_runtime_l1_1_0,
                 0,
                 char **,
                 __cdecl,
                 _get_initial_narrow_environment)
  {
    if (!i::g_initial_u8envp) {
      if (!musl::__environ) {
        _initialize_wide_environment();
        musl::__environ = i::u8envp_from_wenvp(*__p__wenviron());
      }
      i::g_initial_u8envp = i::frozen_envp_copy(musl::__environ);
    }

    if (i::g_initial_u8envp)
      return i::g_initial_u8envp;
    return musl::__environ;
  }

  namespace i
  {
    char **u8envp_from_wenvp(wchar_t **wenvp)
    {
      int envc = 0;
      size_t total_size = 0;
      while (wenvp[envc]) {
        total_size += WideCharToMultiByte(
            CP_UTF8, 0, wenvp[envc], -1, nullptr, 0, nullptr, nullptr);
        envc++;
      }
      total_size += sizeof(char *) * (envc + 1);

      char *block = (char *)malloc(total_size);
      char *end = block + total_size;
      if (!block)
        return nullptr;

      char **u8envp = (char **)block;
      char *u8str = (char *)(u8envp + envc + 1);

      for (int k = 0; k < envc; k++) {
        int size = WideCharToMultiByte(
            CP_UTF8, 0, wenvp[k], -1, u8str, end - u8str, nullptr, nullptr);
        u8envp[k] = u8str;
        u8str += size;
      }
      u8envp[envc] = nullptr;
      return u8envp;
    }

    // musl's setenv mutates existing slots in place, so a deep copy
    // (pointer array + string block) is required for the freeze.
    char **frozen_envp_copy(char **envp)
    {
      if (!envp)
        return nullptr;

      size_t envc = 0;
      size_t total_size = 0;
      for (char **e = envp; *e; e++) {
        envc++;
        total_size += c::strlen(*e) + 1;
      }

      char *block =
          (char *)malloc(sizeof(char *) * (envc + 1) + total_size);
      if (!block)
        return nullptr;

      char **copy = (char **)block;
      char *strings = (char *)(copy + envc + 1);

      for (size_t k = 0; k < envc; k++) {
        size_t len = c::strlen(envp[k]) + 1;
        for (size_t j = 0; j < len; j++)
          strings[j] = envp[k][j];
        copy[k] = strings;
        strings += len;
      }
      copy[envc] = nullptr;
      return copy;
    }
  } // namespace i
} // namespace mingw_thunk
