#pragma once

#include <windows.h>

namespace mingw_thunk
{
  namespace f
  {
    DWORD __stdcall win9x_FormatMessageW(_In_ DWORD dwFlags,
                                         _In_opt_ LPCVOID lpSource,
                                         _In_ DWORD dwMessageId,
                                         _In_ DWORD dwLanguageId,
                                         _Out_ LPWSTR lpBuffer,
                                         _In_ DWORD nSize,
                                         _In_opt_ va_list *Arguments);
  } // namespace f
} // namespace mingw_thunk
