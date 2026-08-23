#include <thunk/_common.h>
#include <thunk/string.h>

#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32,
                 12,
                 HMODULE,
                 WINAPI,
                 LoadLibraryExA,
                 _In_ LPCSTR lpLibFileName,
                 HANDLE hFile,
                 _In_ DWORD dwFlags)
  {
    if (!lpLibFileName) {
      SetLastError(ERROR_INVALID_PARAMETER);
      return nullptr;
    }

    d::w_str w_name;
    if (!w_name.from_u(lpLibFileName)) {
      SetLastError(ERROR_OUTOFMEMORY);
      return nullptr;
    }

    return LoadLibraryExW(w_name.data(), hFile, dwFlags);
  }
} // namespace mingw_thunk
