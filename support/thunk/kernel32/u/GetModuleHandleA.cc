#include <thunk/_common.h>
#include <thunk/string.h>

#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32,
                 4,
                 HMODULE,
                 WINAPI,
                 GetModuleHandleA,
                 _In_opt_ LPCSTR lpModuleName)
  {
    d::w_str w_name;
    if (lpModuleName && !w_name.from_u(lpModuleName)) {
      SetLastError(ERROR_OUTOFMEMORY);
      return nullptr;
    }

    return GetModuleHandleW(lpModuleName ? w_name.c_str() : nullptr);
  }
} // namespace mingw_thunk
