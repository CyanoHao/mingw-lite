#include <thunk/_common.h>
#include <thunk/string.h>

#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32,
                 8,
                 BOOL,
                 WINAPI,
                 SetEnvironmentVariableA,
                 _In_ LPCSTR lpName,
                 _In_opt_ LPCSTR lpValue)
  {
    if (!lpName) {
      SetLastError(ERROR_INVALID_PARAMETER);
      return FALSE;
    }

    d::w_str w_name;
    if (!w_name.from_u(lpName)) {
      SetLastError(ERROR_OUTOFMEMORY);
      return FALSE;
    }

    d::w_str w_value;
    if (lpValue && !w_value.from_u(lpValue)) {
      SetLastError(ERROR_OUTOFMEMORY);
      return FALSE;
    }

    return SetEnvironmentVariableW(w_name.c_str(),
                                   lpValue ? w_value.c_str() : nullptr);
  }
} // namespace mingw_thunk
