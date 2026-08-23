#include "Module32NextW.h"
#include "thunk/os.h"

#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/module_entry.h>

#include <tlhelp32.h>
#include <windows.h>

namespace mingw_thunk
{
  namespace f
  {
    BOOL WINAPI winme_Module32NextW(_In_ HANDLE hSnapshot,
                                    _Out_ LPMODULEENTRY32W lpme);
  }

  __DEFINE_THUNK(kernel32,
                 8,
                 BOOL,
                 WINAPI,
                 Module32NextW,
                 _In_ HANDLE hSnapshot,
                 _Out_ LPMODULEENTRY32W lpme)
  {
    if (i::is_nt()) {
      __DISPATCH_THUNK_2(Module32NextW,
                         const auto pfn = try_get_Module32NextW(),
                         pfn,
                         &f::noop_Module32NextW);
    } else {
      __DISPATCH_THUNK_2(Module32NextW,
                         const auto pfn = kernel32_Module32Next(),
                         &f::winme_Module32NextW,
                         &f::noop_Module32NextW);
    }

    return dllimport_Module32NextW(hSnapshot, lpme);
  }

  namespace f
  {
    BOOL WINAPI winme_Module32NextW(_In_ HANDLE hSnapshot,
                                    _Out_ LPMODULEENTRY32W lpme)
    {
      static const auto pfn = kernel32_Module32Next();

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

    BOOL WINAPI noop_Module32NextW(_In_ HANDLE hSnapshot,
                                   _Out_ LPMODULEENTRY32W lpme)
    {
      SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
      return FALSE;
    }
  } // namespace f
} // namespace mingw_thunk
