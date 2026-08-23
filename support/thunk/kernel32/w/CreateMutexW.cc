#include "CreateMutexW.h"

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
                 CreateMutexW,
                 _In_opt_ LPSECURITY_ATTRIBUTES lpMutexAttributes,
                 _In_ BOOL bInitialOwner,
                 _In_opt_ LPCWSTR lpName)
  {
    __DISPATCH_THUNK_2(
        CreateMutexW, i::is_nt(), &__ms_CreateMutexW, &f::win9x_CreateMutexW);

    return dllimport_CreateMutexW(lpMutexAttributes, bInitialOwner, lpName);
  }

  namespace f
  {
    HANDLE __stdcall
    win9x_CreateMutexW(_In_opt_ LPSECURITY_ATTRIBUTES lpMutexAttributes,
                       _In_ BOOL bInitialOwner,
                       _In_opt_ LPCWSTR lpName)
    {
      d::a_str a_name;
      if (lpName && !a_name.from_w(lpName)) {
        SetLastError(ERROR_OUTOFMEMORY);
        return NULL;
      }

      return __ms_CreateMutexA(
          lpMutexAttributes, bInitialOwner, lpName ? a_name.c_str() : nullptr);
    }
  } // namespace f
} // namespace mingw_thunk
