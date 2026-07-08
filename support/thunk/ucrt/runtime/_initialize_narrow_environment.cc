#include <thunk/_common.h>
#include <thunk/utf8-musl.h>

#include <corecrt_startup.h>
#include <stdlib.h>

#include <unistd.h>
#include <windows.h>

namespace mingw_thunk
{
  namespace i
  {
    char **u8envp_from_wenvp(wchar_t **wenvp);
  } // namespace i

  __DEFINE_THUNK(api_ms_win_crt_runtime_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 _initialize_narrow_environment)
  {
    // BEWARE: runtime is not fully initialized yet!
    _initialize_wide_environment();
    musl::__environ = i::u8envp_from_wenvp(*__p__wenviron());
    return 0;
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

      for (int i = 0; i < envc; i++) {
        int size = WideCharToMultiByte(
            CP_UTF8, 0, wenvp[i], -1, u8str, end - u8str, nullptr, nullptr);
        u8envp[i] = u8str;
        u8str += size;
      }
      u8envp[envc] = nullptr;
      return u8envp;
    }
  } // namespace i
} // namespace mingw_thunk
