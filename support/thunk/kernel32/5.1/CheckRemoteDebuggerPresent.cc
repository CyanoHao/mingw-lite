#include "CheckRemoteDebuggerPresent.h"

#include <thunk/_common.h>

#include <debugapi.h>
#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32,
                 8,
                 BOOL,
                 WINAPI,
                 CheckRemoteDebuggerPresent,
                 _In_ HANDLE hProcess,
                 _Inout_ PBOOL pbDebuggerPresent)
  {
    __DISPATCH_THUNK_2(CheckRemoteDebuggerPresent,
                       const auto pfn = try_get_CheckRemoteDebuggerPresent(),
                       pfn,
                       &f::fallback_CheckRemoteDebuggerPresent);

    return dllimport_CheckRemoteDebuggerPresent(hProcess, pbDebuggerPresent);
  }

  namespace f
  {
    BOOL WINAPI fallback_CheckRemoteDebuggerPresent(
        _In_ HANDLE hProcess, _Inout_ PBOOL pbDebuggerPresent)
    {
      *pbDebuggerPresent =
          (hProcess == GetCurrentProcess()) && IsDebuggerPresent();
      return TRUE;
    }
  } // namespace f
} // namespace mingw_thunk
