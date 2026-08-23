#include <thunk/_common.h>
#include <thunk/module_entry.h>

#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32,
                 8,
                 BOOL,
                 WINAPI,
                 Module32First,
                 _In_ HANDLE hSnapshot,
                 _Inout_ LPMODULEENTRY32 lpme)
  {
    if (!lpme || lpme->dwSize < sizeof(MODULEENTRY32)) {
      SetLastError(ERROR_INVALID_PARAMETER);
      return FALSE;
    }

    MODULEENTRY32W wide{};
    wide.dwSize = sizeof(wide);

    if (!Module32FirstW(hSnapshot, &wide))
      return FALSE;

    DWORD dwSize = lpme->dwSize;
    *lpme = i::w2u(wide);
    lpme->dwSize = dwSize;
    return TRUE;
  }
} // namespace mingw_thunk
