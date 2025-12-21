#include <thunk/_common.h>

#include <windows.h>

#include "../environment/__p__environ.h"

extern "C"
{
  struct _startupinfo;
  _CRTIMP int __cdecl __getmainargs(int *_Argc,
                                    char ***_Argv,
                                    char ***_Env,
                                    int _DoWildCard,
                                    _startupinfo *_StartInfo);
  _CRTIMP int __cdecl __wgetmainargs(int *_Argc,
                                     wchar_t ***_Argv,
                                     wchar_t ***_Env,
                                     int _DoWildCard,
                                     _startupinfo *_StartInfo);
}

namespace mingw_thunk
{
  __DEFINE_THUNK(msvcrt,
                 0,
                 int,
                 __cdecl,
                 __getmainargs,
                 int *pargc,
                 char ***pargv,
                 char ***penvp,
                 int do_wild_card,
                 _startupinfo *start_info)
  {
    // BEWARE: runtime is not fully initialized yet!
    int argc;
    wchar_t **wargv;
    wchar_t **wenvp;

    __wgetmainargs(&argc, &wargv, &wenvp, do_wild_card, start_info);

    char **argv = (char **)malloc(sizeof(char *) * (argc + 1));
    for (int i = 0; i < argc; i++) {
      int len = WideCharToMultiByte(
          CP_UTF8, 0, wargv[i], -1, nullptr, 0, nullptr, nullptr);
      argv[i] = (char *)malloc(len);
      WideCharToMultiByte(
          CP_UTF8, 0, wargv[i], -1, argv[i], len, nullptr, nullptr);
    }
    argv[argc] = nullptr;

    int envc = 0;
    while (wenvp[envc])
      envc++;

    char **envp = (char **)malloc(sizeof(char *) * (envc + 1));
    for (int i = 0; i < envc; i++) {
      int len = WideCharToMultiByte(
          CP_UTF8, 0, wenvp[i], -1, nullptr, 0, nullptr, nullptr);
      envp[i] = (char *)malloc(len);
      WideCharToMultiByte(
          CP_UTF8, 0, wenvp[i], -1, envp[i], len, nullptr, nullptr);
    }
    envp[envc] = nullptr;

    __atomic_test_and_set(&internal::u8_environ_lock, __ATOMIC_ACQUIRE);
    free(internal::u8_environ);
    internal::u8_environ = envp;
    internal::u8_environ_size = envc;
    __atomic_clear(&internal::u8_environ_lock, __ATOMIC_RELEASE);

    *pargc = argc;
    *pargv = argv;
    *penvp = envp;

    return 0;
  }

  // upstream added this to fix msvcrt32 ABI
  __DECLARE_FORCE_OVERRIDE_MINGW_EMU(__getmainargs)
} // namespace mingw_thunk
