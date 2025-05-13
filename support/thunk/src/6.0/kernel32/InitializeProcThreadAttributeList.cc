#include <thunk/_common.h>
#include <thunk/yy/api-ms-win-core-processthreads.h>

#include <processthreadsapi.h>
#include <windows.h>

namespace mingw_thunk
{
  // YY-Thunks 1.1.6
  // Minimum supported client	Windows Vista [desktop apps only]
  // Minimum supported server	Windows Server 2008 [desktop apps only]
  __DEFINE_THUNK(kernel32,
                 16,
                 _Success_(return != FALSE) BOOL,
                 WINAPI,
                 InitializeProcThreadAttributeList,
                 _Out_writes_bytes_to_opt_(*lpSize, *lpSize)
                     LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList,
                 _In_ DWORD dwAttributeCount,
                 _Reserved_ DWORD dwFlags,
                 _When_(lpAttributeList == nullptr, _Out_)
                     _When_(lpAttributeList != nullptr, _Inout_) PSIZE_T lpSize)
  {
    if (const auto pInitializeProcThreadAttributeList =
            try_get_InitializeProcThreadAttributeList()) {
      return pInitializeProcThreadAttributeList(
          lpAttributeList, dwAttributeCount, dwFlags, lpSize);
    }

    LSTATUS lStatus = ERROR_SUCCESS;

    do {
      // 参数验证
      if (dwFlags) {
        lStatus = ERROR_INVALID_PARAMETER;
        break;
      }

      // Vista只支持 3个 Type，所以只有三个
      if (dwAttributeCount > 3) {
        lStatus = ERROR_INVALID_PARAMETER;
        break;
      }

      const auto cbSize = *lpSize;
      const auto cbSizeNeed =
          sizeof(PROC_THREAD_ATTRIBUTE_LIST) +
          dwAttributeCount * sizeof(PROC_THREAD_ATTRIBUTE_ENTRY);
      *lpSize = cbSizeNeed;

      if (lpAttributeList == nullptr || cbSize < cbSizeNeed) {
        lStatus = ERROR_INSUFFICIENT_BUFFER;
        break;
      }

      lpAttributeList->dwFlags = 0;
      lpAttributeList->lpExtendedFlags = nullptr;
      lpAttributeList->Size = dwAttributeCount;
      lpAttributeList->Count = 0;

      return TRUE;

    } while (false);

    SetLastError(lStatus);
    return FALSE;
  }
} // namespace mingw_thunk
