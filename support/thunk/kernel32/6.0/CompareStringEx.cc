#include "CompareStringEx.h"

#include <thunk/_common.h>

#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32,
                 36,
                 int,
                 WINAPI,
                 CompareStringEx,
                 _In_opt_ LPCWSTR lpLocaleName,
                 _In_ DWORD dwCmpFlags,
                 _In_ LPCWSTR lpString1,
                 _In_ int cchCount1,
                 _In_ LPCWSTR lpString2,
                 _In_ int cchCount2,
                 _In_opt_ LPNLSVERSIONINFO lpVersionInformation,
                 _In_opt_ LPVOID lpReserved,
                 _In_opt_ LPARAM lParam)
  {
    __DISPATCH_THUNK_2(CompareStringEx,
                       const auto pfn = try_get_CompareStringEx(),
                       pfn,
                       &f::fallback_CompareStringEx);

    return dllimport_CompareStringEx(lpLocaleName,
                                     dwCmpFlags,
                                     lpString1,
                                     cchCount1,
                                     lpString2,
                                     cchCount2,
                                     lpVersionInformation,
                                     lpReserved,
                                     lParam);
  }

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
                             _In_opt_ LPARAM lParam)
    {
      return CompareStringW(LOCALE_USER_DEFAULT,
                            dwCmpFlags,
                            lpString1,
                            cchCount1,
                            lpString2,
                            cchCount2);
    }
  } // namespace f
} // namespace mingw_thunk
