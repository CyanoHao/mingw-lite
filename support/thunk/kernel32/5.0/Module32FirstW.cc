#include "Module32FirstW.h"

#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/module_entry.h>
#include <thunk/os.h>

#include <tlhelp32.h>
#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32,
                 8,
                 BOOL,
                 WINAPI,
                 Module32FirstW,
                 _In_ HANDLE hSnapshot,
                 _Inout_ LPMODULEENTRY32W lpme)
  {
    if (i::is_nt()) {
      __DISPATCH_THUNK_2(Module32FirstW,
                         const auto pfn = try_get_Module32FirstW(),
                         pfn,
                         &f::noop_Module32FirstW);
    } else {
      __DISPATCH_THUNK_2(Module32FirstW,
                         const auto pfn = kernel32_Module32First(),
                         &f::winme_Module32FirstW,
                         &f::noop_Module32FirstW);
    }

    return dllimport_Module32FirstW(hSnapshot, lpme);
  }

  namespace f
  {
    BOOL WINAPI winme_Module32FirstW(_In_ HANDLE hSnapshot,
                                     _Inout_ LPMODULEENTRY32W lpme)
    {
      static const auto pfn = kernel32_Module32First();

      if (!lpme || lpme->dwSize < sizeof(MODULEENTRY32W)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
      }

      MODULEENTRY32 narrow = {.dwSize = sizeof(narrow)};

      if (!pfn(hSnapshot, &narrow))
        return FALSE;

      DWORD dwSize = lpme->dwSize;
      *lpme = i::a2w(narrow);
      lpme->dwSize = dwSize;
      return TRUE;
    }

    BOOL WINAPI noop_Module32FirstW(_In_ HANDLE hSnapshot,
                                    _Inout_ LPMODULEENTRY32W lpme)
    {
      SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
      return FALSE;
    }
  } // namespace f
} // namespace mingw_thunk
