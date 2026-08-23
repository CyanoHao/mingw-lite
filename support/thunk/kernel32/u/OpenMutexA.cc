#include <thunk/_common.h>
#include <thunk/string.h>

#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32,
                 12,
                 HANDLE,
                 WINAPI,
                 OpenMutexA,
                 _In_ DWORD dwDesiredAccess,
                 _In_ BOOL bInheritHandle,
                 _In_ LPCSTR lpName)
  {
    d::w_str w_name;
    if (!w_name.from_u(lpName)) {
      SetLastError(ERROR_OUTOFMEMORY);
      return nullptr;
    }

    return OpenMutexW(dwDesiredAccess, bInheritHandle, w_name.c_str());
  }
} // namespace mingw_thunk
