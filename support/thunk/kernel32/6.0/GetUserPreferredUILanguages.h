#pragma once

#include <windows.h>

// after <windows.h>
#include <winnls.h>

namespace mingw_thunk
{
  namespace f
  {
    BOOL WINAPI
    fallback_GetUserPreferredUILanguages(_In_ DWORD dwFlags,
                                         _Out_ PULONG pulNumLanguages,
                                         _Out_opt_ PZZWSTR pwszLanguagesBuffer,
                                         _Inout_ PULONG pcchLanguagesBuffer);
  }
} // namespace mingw_thunk
