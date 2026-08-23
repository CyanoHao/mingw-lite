#pragma once

#include <windows.h>

namespace mingw_thunk
{
  namespace f
  {
    WINBOOL __stdcall
    win9x_GetVolumeInformationW(_In_opt_ LPCWSTR lpRootPathName,
                                _Out_opt_ LPWSTR lpVolumeNameBuffer,
                                _In_ DWORD nVolumeNameSize,
                                _Out_opt_ LPDWORD lpVolumeSerialNumber,
                                _Out_opt_ LPDWORD lpMaximumComponentLength,
                                _Out_opt_ LPDWORD lpFileSystemFlags,
                                _Out_opt_ LPWSTR lpFileSystemNameBuffer,
                                _In_ DWORD nFileSystemNameSize);
  } // namespace f
} // namespace mingw_thunk
