#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/os.h>

#include <stdlib.h>

#include <windows.h>

#include "../environment/__p__environ.h"
#include "__p___argv.h"

extern "C"
{
  struct _startupinfo;
  _CRTIMP int __cdecl __wgetmainargs(int *_Argc,
                                     wchar_t ***_Argv,
                                     wchar_t ***_Env,
                                     int _DoWildCard,
                                     _startupinfo *_StartInfo);
}

namespace mingw_thunk
{
  namespace i
  {
    struct u8_envp_result
    {
      int envc;
      char **envp;
    };

    u8_envp_result u8_envp_from_wenvp(wchar_t **wenvp);
  } // namespace i

  __DEFINE_THUNK(crtdll,
                 0,
                 void,
                 __cdecl,
                 __getmainargs,
                 int *pargc,
                 char ***pargv,
                 char ***penvp,
                 int do_wild_card,
                 _startupinfo *stratup_info)
  {
    // BEWARE: runtime is not fully initialized yet!
    int argc;
    wchar_t **wargv;
    wchar_t **wenvp;

    __wgetmainargs(&argc, &wargv, &wenvp, do_wild_card, nullptr);

    char **argv = (char **)malloc(sizeof(char *) * (argc + 1));
    for (int i = 0; i < argc; i++) {
      int len = WideCharToMultiByte(
          CP_UTF8, 0, wargv[i], -1, nullptr, 0, nullptr, nullptr);
      argv[i] = (char *)malloc(len);
      WideCharToMultiByte(
          CP_UTF8, 0, wargv[i], -1, argv[i], len, nullptr, nullptr);
    }
    argv[argc] = nullptr;

    auto ret = i::u8_envp_from_wenvp(wenvp);

    i::u8_argv = argv;
    i::u8_envp = ret.envp;

    __atomic_test_and_set(&i::u8_environ_lock, __ATOMIC_ACQUIRE);
    i::u8_environ = ret.envp;
    i::u8_environ_size = ret.envc;
    __atomic_clear(&i::u8_environ_lock, __ATOMIC_RELEASE);

    *pargc = argc;
    *pargv = argv;
    *penvp = ret.envp;
  }

  // upstream added this to fix msvcrt32 ABI
  __DECLARE_FORCE_OVERRIDE_MINGW_EMU(__getmainargs)

  namespace i
  {
    u8_envp_result u8_envp_from_wenvp(wchar_t **wenvp)
    {
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

      return {envc, envp};
    }
  } // namespace i
} // namespace mingw_thunk
