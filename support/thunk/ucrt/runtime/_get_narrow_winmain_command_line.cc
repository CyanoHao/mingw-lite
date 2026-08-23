#include <thunk/_common.h>

#include <stdlib.h>

#include <windows.h>

namespace mingw_thunk
{
  // WinMain-style narrow command line: GetCommandLineW minus the program
  // token (quoted or bare), transcoded once to UTF-8 (plan-2 M7; wine
  // anchor: empty string for an argument-less run).
  __DEFINE_THUNK(api_ms_win_crt_runtime_l1_1_0,
                 0,
                 char *,
                 __cdecl,
                 _get_narrow_winmain_command_line)
  {
    static char *cache = nullptr;
    if (!cache) {
      const wchar_t *cmd = GetCommandLineW();

      if (*cmd == L'"') {
        ++cmd;
        while (*cmd && *cmd != L'"')
          ++cmd;
        if (*cmd)
          ++cmd;
      } else {
        while (*cmd && *cmd != L' ' && *cmd != L'\t')
          ++cmd;
      }
      while (*cmd == L' ' || *cmd == L'\t')
        ++cmd;

      int need = WideCharToMultiByte(
          CP_UTF8, 0, cmd, -1, nullptr, 0, nullptr, nullptr);
      char *converted = (char *)malloc(need > 0 ? size_t(need) : size_t(1));
      if (!converted) {
        static char empty[] = "";
        cache = empty;
        return cache;
      }
      WideCharToMultiByte(
          CP_UTF8, 0, cmd, -1, converted, need, nullptr, nullptr);
      cache = converted;
    }
    return cache;
  }
} // namespace mingw_thunk
