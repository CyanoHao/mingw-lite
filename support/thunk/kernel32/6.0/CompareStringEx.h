#pragma once

#include <windows.h>

namespace mingw_thunk
{
  namespace f
  {
    int WINAPI
    fallback_CompareStringEx(_In_opt_ LPCWSTR lpLocaleName,
                             _In_ DWORD dwCmpFlags,
                             _In_ LPCWSTR lpString1,
                             _In_ int cchCount1,
                             _In_ LPCWSTR lpString2,
                             _In_ int cchCount2,
                             _In_opt_ LPNLSVERSIONINFO lpVersionInformation,
                             _In_opt_ LPVOID lpReserved,
                             _In_opt_ LPARAM lParam);
  }
} // namespace mingw_thunk
