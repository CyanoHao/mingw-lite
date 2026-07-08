#include <thunk/_common.h>
#include <thunk/utf8-musl.h>

#include <corecrt_startup.h>
#include <stdlib.h>

#include <windows.h>

namespace mingw_thunk
{
  namespace i
  {
    char **u8argv_from_wargv(int argc, wchar_t **wargv);
  } // namespace i

  __DEFINE_THUNK(api_ms_win_crt_runtime_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 _configure_narrow_argv,
                 _crt_argv_mode mode)
  {
    // BEWARE: runtime is not fully initialized yet!
    _configure_wide_argv(mode);
    musl::utf8_argv = i::u8argv_from_wargv(*__p___argc(), *__p___wargv());
    return 0;
  }

  namespace i
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
  } // namespace i
} // namespace mingw_thunk
