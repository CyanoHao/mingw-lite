#include "GetVolumeInformationW.h"

#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/os.h>
#include <thunk/string.h>

#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32,
                 32,
                 WINBOOL,
                 WINAPI,
                 GetVolumeInformationW,
                 _In_opt_ LPCWSTR lpRootPathName,
                 _Out_opt_ LPWSTR lpVolumeNameBuffer,
                 _In_ DWORD nVolumeNameSize,
                 _Out_opt_ LPDWORD lpVolumeSerialNumber,
                 _Out_opt_ LPDWORD lpMaximumComponentLength,
                 _Out_opt_ LPDWORD lpFileSystemFlags,
                 _Out_opt_ LPWSTR lpFileSystemNameBuffer,
                 _In_ DWORD nFileSystemNameSize)
  {
    __DISPATCH_THUNK_2(GetVolumeInformationW,
                       i::is_nt(),
                       &__ms_GetVolumeInformationW,
                       &f::win9x_GetVolumeInformationW);

    return dllimport_GetVolumeInformationW(lpRootPathName,
                                           lpVolumeNameBuffer,
                                           nVolumeNameSize,
                                           lpVolumeSerialNumber,
                                           lpMaximumComponentLength,
                                           lpFileSystemFlags,
                                           lpFileSystemNameBuffer,
                                           nFileSystemNameSize);
  }

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
                                _In_ DWORD nFileSystemNameSize)
    {
      d::a_str a_root;
      if (lpRootPathName && !a_root.from_w(lpRootPathName)) {
        SetLastError(ERROR_OUTOFMEMORY);
        return FALSE;
      }

      char a_vol[MAX_PATH + 1] = {};
      char a_fs[MAX_PATH + 1] = {};

      if (!__ms_GetVolumeInformationA(lpRootPathName ? a_root.c_str() : nullptr,
                                      lpVolumeNameBuffer ? a_vol : nullptr,
                                      lpVolumeNameBuffer ? MAX_PATH + 1 : 0,
                                      lpVolumeSerialNumber,
                                      lpMaximumComponentLength,
                                      lpFileSystemFlags,
                                      lpFileSystemNameBuffer ? a_fs : nullptr,
                                      lpFileSystemNameBuffer ? MAX_PATH + 1
                                                             : 0))
        return FALSE;

      d::w_str w_vol;
      if (lpVolumeNameBuffer && !w_vol.from_a(a_vol)) {
        SetLastError(ERROR_OUTOFMEMORY);
        return FALSE;
      }

      d::w_str w_fs;
      if (lpFileSystemNameBuffer && !w_fs.from_a(a_fs)) {
        SetLastError(ERROR_OUTOFMEMORY);
        return FALSE;
      }

      if (lpVolumeNameBuffer) {
        if (w_vol.size() + 1 > nVolumeNameSize) {
          SetLastError(ERROR_INSUFFICIENT_BUFFER);
          return FALSE;
        }
      }

      if (lpFileSystemNameBuffer) {
        if (w_fs.size() + 1 > nFileSystemNameSize) {
          SetLastError(ERROR_INSUFFICIENT_BUFFER);
          return FALSE;
        }
      }

      if (lpVolumeNameBuffer) {
        c::wmemcpy(lpVolumeNameBuffer, w_vol.c_str(), w_vol.size());
        lpVolumeNameBuffer[w_vol.size()] = L'\0';
      }

      if (lpFileSystemNameBuffer) {
        c::wmemcpy(lpFileSystemNameBuffer, w_fs.c_str(), w_fs.size());
        lpFileSystemNameBuffer[w_fs.size()] = L'\0';
      }

      return TRUE;
    }
  } // namespace f
} // namespace mingw_thunk
