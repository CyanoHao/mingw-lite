#include <thunk/_common.h>
#include <thunk/string.h>

#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32,
                 32,
                 WINBOOL,
                 WINAPI,
                 GetVolumeInformationA,
                 _In_opt_ LPCSTR lpRootPathName,
                 _Out_opt_ LPSTR lpVolumeNameBuffer,
                 _In_ DWORD nVolumeNameSize,
                 _Out_opt_ LPDWORD lpVolumeSerialNumber,
                 _Out_opt_ LPDWORD lpMaximumComponentLength,
                 _Out_opt_ LPDWORD lpFileSystemFlags,
                 _Out_opt_ LPSTR lpFileSystemNameBuffer,
                 _In_ DWORD nFileSystemNameSize)
  {
    d::w_str w_root;
    if (lpRootPathName && !w_root.from_u(lpRootPathName)) {
      SetLastError(ERROR_OUTOFMEMORY);
      return FALSE;
    }

    WCHAR w_vol[MAX_PATH + 1] = {};
    WCHAR w_fs[MAX_PATH + 1] = {};

    if (!GetVolumeInformationW(lpRootPathName ? w_root.c_str() : nullptr,
                               lpVolumeNameBuffer ? w_vol : nullptr,
                               lpVolumeNameBuffer ? MAX_PATH + 1 : 0,
                               lpVolumeSerialNumber,
                               lpMaximumComponentLength,
                               lpFileSystemFlags,
                               lpFileSystemNameBuffer ? w_fs : nullptr,
                               lpFileSystemNameBuffer ? MAX_PATH + 1 : 0))
      return FALSE;

    d::u_str u_vol;
    if (lpVolumeNameBuffer && !u_vol.from_w(w_vol)) {
      SetLastError(ERROR_OUTOFMEMORY);
      return FALSE;
    }

    d::u_str u_fs;
    if (lpFileSystemNameBuffer && !u_fs.from_w(w_fs)) {
      SetLastError(ERROR_OUTOFMEMORY);
      return FALSE;
    }

    if (lpVolumeNameBuffer) {
      if (u_vol.size() + 1 > nVolumeNameSize) {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
      }
    }

    if (lpFileSystemNameBuffer) {
      if (u_fs.size() + 1 > nFileSystemNameSize) {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
      }
    }

    if (lpVolumeNameBuffer) {
      c::memcpy(lpVolumeNameBuffer, u_vol.c_str(), u_vol.size());
      lpVolumeNameBuffer[u_vol.size()] = '\0';
    }

    if (lpFileSystemNameBuffer) {
      c::memcpy(lpFileSystemNameBuffer, u_fs.c_str(), u_fs.size());
      lpFileSystemNameBuffer[u_fs.size()] = '\0';
    }

    return TRUE;
  }
} // namespace mingw_thunk
