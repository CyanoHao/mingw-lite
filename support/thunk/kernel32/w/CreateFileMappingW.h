#pragma once

#include <windows.h>

namespace mingw_thunk
{
  namespace f
  {
    HANDLE __stdcall win9x_CreateFileMappingW(_In_ HANDLE hFile,
                                              _In_opt_ LPSECURITY_ATTRIBUTES
                                                  lpFileMappingAttributes,
                                              _In_ DWORD flProtect,
                                              _In_ DWORD dwMaximumSizeHigh,
                                              _In_ DWORD dwMaximumSizeLow,
                                              _In_opt_ LPCWSTR lpName);
  }
} // namespace mingw_thunk
