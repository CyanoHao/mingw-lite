#include "GetModuleHandleW.h"

#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/os.h>
#include <thunk/string.h>

#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32,
                 4,
                 HMODULE,
                 WINAPI,
                 GetModuleHandleW,
                 _In_opt_ LPCWSTR lpModuleName)
  {
    __DISPATCH_THUNK_2(GetModuleHandleW,
                       i::is_nt(),
                       &__ms_GetModuleHandleW,
                       &f::win9x_GetModuleHandleW);

    return dllimport_GetModuleHandleW(lpModuleName);
  }

  namespace f
  {
    HMODULE __stdcall win9x_GetModuleHandleW(_In_opt_ LPCWSTR lpModuleName)
    {
      d::a_str a_name;
      if (lpModuleName && !a_name.from_w(lpModuleName)) {
        SetLastError(ERROR_OUTOFMEMORY);
        return NULL;
      }

      return __ms_GetModuleHandleA(lpModuleName ? a_name.c_str() : nullptr);
    }
  } // namespace f
} // namespace mingw_thunk
