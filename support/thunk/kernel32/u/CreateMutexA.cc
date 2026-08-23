#include <thunk/_common.h>
#include <thunk/string.h>

#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32,
                 12,
                 HANDLE,
                 WINAPI,
                 CreateMutexA,
                 _In_opt_ LPSECURITY_ATTRIBUTES lpMutexAttributes,
                 _In_ BOOL bInitialOwner,
                 _In_opt_ LPCSTR lpName)
  {
    d::w_str w_name;
    if (lpName && !w_name.from_u(lpName)) {
      SetLastError(ERROR_OUTOFMEMORY);
      return nullptr;
    }

    return CreateMutexW(
        lpMutexAttributes, bInitialOwner, lpName ? w_name.c_str() : nullptr);
  }
} // namespace mingw_thunk
