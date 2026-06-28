#include "GetProcessId.h"

#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/yy/yy.h>

#include <ntstatus.h>
#include <windows.h>
#include <winternl.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32, 4, DWORD, WINAPI, GetProcessId, _In_ HANDLE Process)
  {
    __DISPATCH_THUNK_2(GetProcessId,
                       const auto pfn = try_get_GetProcessId(),
                       pfn,
                       &f::fallback_GetProcessId);

    return dllimport_GetProcessId(Process);
  }

  namespace f
  {
    DWORD WINAPI fallback_GetProcessId(_In_ HANDLE Process)
    {
      PROCESS_BASIC_INFORMATION info;
      NTSTATUS status = __ms_NtQueryInformationProcess(
          Process, ProcessBasicInformation, &info, sizeof(info), nullptr);
      if (NT_SUCCESS(status))
        return (DWORD)info.UniqueProcessId;

      internal::BaseSetLastNTError(status);
      return 0;
    }
  } // namespace f
} // namespace mingw_thunk
