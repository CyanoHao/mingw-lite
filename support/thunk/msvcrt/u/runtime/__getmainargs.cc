#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/os.h>
#include <thunk/utf8-musl.h>

#include <stdlib.h>

#include <windows.h>

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
  namespace i
  {
    char **u8argv_from_wargv(int argc, wchar_t **wargv);

    char **u8envp_from_wenvp(wchar_t **wenvp);
    char **u8envp_from_win32_env_strings();
  } // namespace i

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

    char **u8argv = i::u8argv_from_wargv(argc, wargv);
    char **u8envp;

#if THUNK_LEVEL >= NTDDI_VISTA

    u8envp = i::u8envp_from_wenvp(wenvp);

#elif THUNK_LEVEL >= NTDDI_WIN4

    if (i::os_version() >= g::win32_vista) {
      u8envp = i::u8envp_from_wenvp(wenvp);
    } else {
      // Fake wenvp (converted from envp)
      u8envp = i::u8envp_from_win32_env_strings();
    }

#else

    if (i::os_version() >= g::win32_vista) {
      u8envp = i::u8envp_from_wenvp(wenvp);
    } else if (i::is_nt()) {
      // Fake wenvp (converted from envp)
      u8envp = i::u8envp_from_win32_env_strings();
    } else {
      // Fake wenvp is what we need:
      // 1. `GetEnvironmentStringsW` is a stub;
      // 2. environment variables are limited to the code page.
      u8envp = i::u8envp_from_wenvp(wenvp);
    }

#endif

    musl::utf8_argv = u8argv;
    musl::__environ = u8envp;

    *pargc = argc;
    *pargv = u8argv;
    *penvp = u8envp;

    return 0;
  }

  // upstream added this to fix msvcrt32 ABI
  __DECLARE_FORCE_OVERRIDE_MINGW_EMU(__getmainargs)

  namespace internal
  {
    char **u8argv_from_wargv(int argc, wchar_t **wargv)
    {
      size_t total_size = sizeof(char *) * (argc + 1);
      for (int i = 0; i < argc; i++) {
        total_size += WideCharToMultiByte(
            CP_UTF8, 0, wargv[i], -1, nullptr, 0, nullptr, nullptr);
      }

      char *block = (char *)malloc(total_size);
      char *end = block + total_size;
      if (!block)
        return nullptr;

      char **u8argv = (char **)block;
      char *u8str = (char *)(u8argv + argc + 1);

      for (int i = 0; i < argc; i++) {
        int len = WideCharToMultiByte(
            CP_UTF8, 0, wargv[i], -1, u8str, end - u8str, nullptr, nullptr);
        u8argv[i] = u8str;
        u8str += len;
      }
      u8argv[argc] = nullptr;
      return u8argv;
    }

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

      for (int i = 0; i < envc; i++) {
        int size = WideCharToMultiByte(
            CP_UTF8, 0, wenvp[i], -1, u8str, end - u8str, nullptr, nullptr);
        u8envp[i] = u8str;
        u8str += size;
      }
      u8envp[envc] = nullptr;
      return u8envp;
    }

    char **u8_envp_from_win32_env_strings()
    {
      wchar_t *env_strings = GetEnvironmentStringsW();
      if (!env_strings)
        return nullptr;

      int envc = 0;
      size_t total_size = 0;

      wchar_t *p = env_strings;
      while (*p) {
        size_t w_size = wcslen(p) + 1;
        total_size += WideCharToMultiByte(
            CP_UTF8, 0, p, w_size, nullptr, 0, nullptr, nullptr);
        envc++;
        p += w_size;
      }
      total_size += sizeof(char *) * (envc + 1);

      char *block = (char *)malloc(total_size);
      char *end = block + total_size;
      if (!block)
        return nullptr;

      char **u8envp = (char **)block;
      char *u8str = (char *)(u8envp + envc + 1);

      p = env_strings;
      for (int i = 0; i < envc; i++) {
        size_t wsize = wcslen(p) + 1;
        int size = WideCharToMultiByte(
            CP_UTF8, 0, p, wsize, u8str, end - u8str, nullptr, nullptr);
        u8envp[i] = u8str;
        u8str += size;
        p += wsize;
      }
      u8envp[envc] = nullptr;

      FreeEnvironmentStringsW(env_strings);
      return u8envp;
    }
  } // namespace internal
} // namespace mingw_thunk
