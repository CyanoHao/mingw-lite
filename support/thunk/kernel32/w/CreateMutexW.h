#pragma once

#include <windows.h>

namespace mingw_thunk
{
  namespace f
  {
    HANDLE __stdcall
    win9x_CreateMutexW(_In_opt_ LPSECURITY_ATTRIBUTES lpMutexAttributes,
                       _In_ BOOL bInitialOwner,
                       _In_opt_ LPCWSTR lpName);
  }
} // namespace mingw_thunk
