#pragma once

#include <windows.h>

namespace mingw_thunk
{
  namespace f
  {
    BOOL WINAPI fallback_CheckRemoteDebuggerPresent(
        _In_ HANDLE hProcess, _Inout_ PBOOL pbDebuggerPresent);
  }
} // namespace mingw_thunk
