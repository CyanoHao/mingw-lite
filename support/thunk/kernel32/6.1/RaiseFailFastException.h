#pragma once

#include <windows.h>

namespace mingw_thunk
{
  namespace f
  {
    void WINAPI
    fallback_RaiseFailFastException(_In_opt_ PEXCEPTION_RECORD pExceptionRecord,
                                    _In_opt_ PCONTEXT pContextRecord,
                                    _In_ DWORD dwFlags);
  }
} // namespace mingw_thunk
