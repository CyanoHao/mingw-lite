#pragma once

#include <windows.h>

namespace mingw_thunk
{
  namespace f
  {
    DWORD WINAPI fallback_GetProcessId(_In_ HANDLE Process);
  }
} // namespace mingw_thunk
