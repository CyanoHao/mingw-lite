#pragma once

#include <windows.h>

namespace mingw_thunk
{
  namespace f
  {
    HANDLE __stdcall win9x_OpenFileMappingW(_In_ DWORD dwDesiredAccess,
                                            _In_ BOOL bInheritHandle,
                                            _In_ LPCWSTR lpName);
  }
} // namespace mingw_thunk
