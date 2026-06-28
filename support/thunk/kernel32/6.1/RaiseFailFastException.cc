#include "RaiseFailFastException.h"

#include <thunk/_common.h>

#include <ntstatus.h>
#include <windows.h>
#include <winternl.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32,
                 12,
                 void,
                 WINAPI,
                 RaiseFailFastException,
                 _In_opt_ PEXCEPTION_RECORD pExceptionRecord,
                 _In_opt_ PCONTEXT pContextRecord,
                 _In_ DWORD dwFlags)
  {
    __DISPATCH_THUNK_2(RaiseFailFastException,
                       const auto pfn = try_get_RaiseFailFastException(),
                       pfn,
                       &f::fallback_RaiseFailFastException);

    dllimport_RaiseFailFastException(pExceptionRecord, pContextRecord, dwFlags);
    return;
  }

  namespace f
  {
    void WINAPI
    fallback_RaiseFailFastException(_In_opt_ PEXCEPTION_RECORD pExceptionRecord,
                                    _In_opt_ PCONTEXT pContextRecord,
                                    _In_ DWORD dwFlags)
    {
      TerminateProcess(GetCurrentProcess(),
                       pExceptionRecord ? pExceptionRecord->ExceptionCode
                                        : STATUS_FAIL_FAST_EXCEPTION);
    }
  } // namespace f
} // namespace mingw_thunk
