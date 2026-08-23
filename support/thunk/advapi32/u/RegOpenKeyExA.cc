#include <thunk/_common.h>
#include <thunk/string.h>

#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(advapi32,
                 20,
                 LSTATUS,
                 APIENTRY,
                 RegOpenKeyExA,
                 _In_ HKEY hKey,
                 _In_ LPCSTR lpSubKey,
                 _In_opt_ DWORD ulOptions,
                 _In_ REGSAM samDesired,
                 _Out_ PHKEY phkResult)
  {
    if (!lpSubKey)
      return ERROR_INVALID_PARAMETER;

    d::w_str w_sub_key;
    if (!w_sub_key.from_u(lpSubKey))
      return ERROR_OUTOFMEMORY;

    return RegOpenKeyExW(
        hKey, w_sub_key.c_str(), ulOptions, samDesired, phkResult);
  }
} // namespace mingw_thunk
