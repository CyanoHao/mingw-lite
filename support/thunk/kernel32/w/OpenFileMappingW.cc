#include "OpenFileMappingW.h"

#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/os.h>
#include <thunk/string.h>

#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32,
                 12,
                 HANDLE,
                 WINAPI,
                 OpenFileMappingW,
                 _In_ DWORD dwDesiredAccess,
                 _In_ BOOL bInheritHandle,
                 _In_ LPCWSTR lpName)
  {
    __DISPATCH_THUNK_2(OpenFileMappingW,
                       i::is_nt(),
                       &__ms_OpenFileMappingW,
                       &f::win9x_OpenFileMappingW);

    return dllimport_OpenFileMappingW(dwDesiredAccess, bInheritHandle, lpName);
  }

  namespace f
  {
    HANDLE __stdcall win9x_OpenFileMappingW(_In_ DWORD dwDesiredAccess,
                                            _In_ BOOL bInheritHandle,
                                            _In_ LPCWSTR lpName)
    {
      d::a_str a_name;
      if (!a_name.from_w(lpName)) {
        SetLastError(ERROR_OUTOFMEMORY);
        return NULL;
      }

      return __ms_OpenFileMappingA(
          dwDesiredAccess, bInheritHandle, a_name.c_str());
    }
  } // namespace f
} // namespace mingw_thunk
