#include <thunk/_common.h>

#include <stdlib.h>

#include <windows.h>

namespace mingw_thunk
{
  namespace i
  {
    // Single lazily-built UTF-8 cache behind the __p__pgmptr slot
    // (plan-2 M7, the last P0): same mechanism as __p___argv, different
    // value source — the real module path, not the CreateProcess-given
    // argv[0].  On GetModuleFileNameW failure the slot stays NULL
    // (native parity: _pgmptr keeps its zero-initialized value).
    char **pgmptr_slot()
    {
      static char *cache = nullptr;
      if (!cache) {
        wchar_t w_path[MAX_PATH];
        DWORD n = GetModuleFileNameW(nullptr, w_path, MAX_PATH);
        if (n > 0 && n < MAX_PATH) {
          static char u8_path[MAX_PATH * 4];
          int len = WideCharToMultiByte(CP_UTF8,
                                        0,
                                        w_path,
                                        (int)n,
                                        u8_path,
                                        sizeof(u8_path) - 1,
                                        nullptr,
                                        nullptr);
          if (len > 0) {
            u8_path[len] = 0;
            cache = u8_path;
          }
        }
      }
      return &cache;
    }
  } // namespace i

  __DEFINE_THUNK(
      api_ms_win_crt_runtime_l1_1_0, 0, char **, __cdecl, __p__pgmptr)
  {
    return i::pgmptr_slot();
  }
} // namespace mingw_thunk
