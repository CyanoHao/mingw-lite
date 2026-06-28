#include "GetNativeSystemInfo.h"

#include <thunk/_common.h>

#include <sysinfoapi.h>
#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32,
                 4,
                 void,
                 WINAPI,
                 GetNativeSystemInfo,
                 _Out_ LPSYSTEM_INFO lpSystemInfo)
  {
    __DISPATCH_THUNK_2(GetNativeSystemInfo,
                       const auto pfn = try_get_GetNativeSystemInfo(),
                       pfn,
                       &f::fallback_GetNativeSystemInfo);

    dllimport_GetNativeSystemInfo(lpSystemInfo);
    return;
  }

  namespace f
  {
    void WINAPI fallback_GetNativeSystemInfo(_Out_ LPSYSTEM_INFO lpSystemInfo)
    {
      GetSystemInfo(lpSystemInfo);
    }
  } // namespace f
} // namespace mingw_thunk
