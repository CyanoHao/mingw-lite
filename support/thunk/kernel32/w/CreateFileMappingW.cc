#include "CreateFileMappingW.h"

#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/os.h>
#include <thunk/string.h>

#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32,
                 24,
                 HANDLE,
                 WINAPI,
                 CreateFileMappingW,
                 _In_ HANDLE hFile,
                 _In_opt_ LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
                 _In_ DWORD flProtect,
                 _In_ DWORD dwMaximumSizeHigh,
                 _In_ DWORD dwMaximumSizeLow,
                 _In_opt_ LPCWSTR lpName)
  {
    __DISPATCH_THUNK_2(CreateFileMappingW,
                       i::is_nt(),
                       &__ms_CreateFileMappingW,
                       &f::win9x_CreateFileMappingW);

    return dllimport_CreateFileMappingW(hFile,
                                        lpFileMappingAttributes,
                                        flProtect,
                                        dwMaximumSizeHigh,
                                        dwMaximumSizeLow,
                                        lpName);
  }

  namespace f
  {
    HANDLE __stdcall win9x_CreateFileMappingW(_In_ HANDLE hFile,
                                              _In_opt_ LPSECURITY_ATTRIBUTES
                                                  lpFileMappingAttributes,
                                              _In_ DWORD flProtect,
                                              _In_ DWORD dwMaximumSizeHigh,
                                              _In_ DWORD dwMaximumSizeLow,
                                              _In_opt_ LPCWSTR lpName)
    {
      d::a_str a_name;
      if (lpName && !a_name.from_w(lpName)) {
        SetLastError(ERROR_OUTOFMEMORY);
        return NULL;
      }

      return __ms_CreateFileMappingA(hFile,
                                     lpFileMappingAttributes,
                                     flProtect,
                                     dwMaximumSizeHigh,
                                     dwMaximumSizeLow,
                                     lpName ? a_name.c_str() : nullptr);
    }
  } // namespace f
} // namespace mingw_thunk
