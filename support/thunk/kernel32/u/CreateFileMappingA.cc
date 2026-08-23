#include <thunk/_common.h>
#include <thunk/string.h>

#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32,
                  24,
                  HANDLE,
                  WINAPI,
                  CreateFileMappingA,
                  _In_ HANDLE hFile,
                  _In_opt_ LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
                  _In_ DWORD flProtect,
                  _In_ DWORD dwMaximumSizeHigh,
                  _In_ DWORD dwMaximumSizeLow,
                  _In_opt_ LPCSTR lpName)
  {
    d::w_str w_name;
    if (lpName && !w_name.from_u(lpName)) {
      SetLastError(ERROR_OUTOFMEMORY);
      return nullptr;
    }

    return CreateFileMappingW(hFile,
                              lpFileMappingAttributes,
                              flProtect,
                              dwMaximumSizeHigh,
                              dwMaximumSizeLow,
                              lpName ? w_name.c_str() : nullptr);
  }
} // namespace mingw_thunk
