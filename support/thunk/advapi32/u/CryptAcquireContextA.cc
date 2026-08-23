#include <thunk/_common.h>
#include <thunk/string.h>

#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(advapi32,
                 20,
                 BOOL,
                 WINAPI,
                 CryptAcquireContextA,
                 _Out_opt_ HCRYPTPROV *phProv,
                 _In_opt_ LPCSTR pszContainer,
                 _In_opt_ LPCSTR pszProvider,
                 _In_ DWORD dwProvType,
                 _In_ DWORD dwFlags)
  {
    d::w_str w_container;
    d::w_str w_provider;
    const wchar_t *container = nullptr;
    const wchar_t *provider = nullptr;

    if (pszContainer) {
      if (!w_container.from_u(pszContainer)) {
        SetLastError(ERROR_OUTOFMEMORY);
        return FALSE;
      }
      container = w_container.c_str();
    }

    if (pszProvider) {
      if (!w_provider.from_u(pszProvider)) {
        SetLastError(ERROR_OUTOFMEMORY);
        return FALSE;
      }
      provider = w_provider.c_str();
    }

    return CryptAcquireContextW(
        phProv, container, provider, dwProvType, dwFlags);
  }
} // namespace mingw_thunk
