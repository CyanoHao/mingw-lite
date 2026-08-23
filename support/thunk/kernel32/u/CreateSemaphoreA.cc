#include <thunk/_common.h>
#include <thunk/string.h>

#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32,
                 16,
                 HANDLE,
                 WINAPI,
                 CreateSemaphoreA,
                 _In_opt_ LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
                 _In_ LONG lInitialCount,
                 _In_ LONG lMaximumCount,
                 _In_opt_ LPCSTR lpName)
  {
    d::w_str w_name;
    if (lpName && !w_name.from_u(lpName)) {
      SetLastError(ERROR_OUTOFMEMORY);
      return nullptr;
    }

    return CreateSemaphoreW(lpSemaphoreAttributes,
                            lInitialCount,
                            lMaximumCount,
                            lpName ? w_name.c_str() : nullptr);
  }
} // namespace mingw_thunk
