#include <thunk/_common.h>
#include <thunk/try_get.h>

#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32,
                 12,
                 BOOL,
                 WINAPI,
                 GetModuleHandleExW,
                 _In_ DWORD dwFlags,
                 _In_opt_ LPCWSTR lpModuleName,
                 _Out_ HMODULE *phModule)
  {
    if (const auto pfn = try_get_GetModuleHandleExW())
      return pfn(dwFlags, lpModuleName, phModule);

    DWORD known_flags = GET_MODULE_HANDLE_EX_FLAG_PIN |
                        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT |
                        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS;

    bool pin = dwFlags & GET_MODULE_HANDLE_EX_FLAG_PIN;
    bool unchanged_refcount =
        dwFlags & GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;
    bool from_address = dwFlags & GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS;

    if ((dwFlags & ~known_flags) || pin ||
        (unchanged_refcount && from_address)) {
      SetLastError(ERROR_NOT_SUPPORTED);
      return FALSE;
    }

    HMODULE h;
    if (from_address) {
      if (const auto pRtlPcToFileHeader = try_get_RtlPcToFileHeader()) {
        h = (HMODULE)pRtlPcToFileHeader((PVOID)lpModuleName, (PVOID *)&h);
        if (!h)
          SetLastError(ERROR_DLL_NOT_FOUND);
      } else {
        SetLastError(ERROR_NOT_SUPPORTED);
        return FALSE;
      }
    } else {
      if (unchanged_refcount)
        h = GetModuleHandleW(lpModuleName);
      else
        h = LoadLibraryW(lpModuleName);
    }
    if (h) {
      *phModule = h;
      return TRUE;
    } else {
      return FALSE;
    }
  }
} // namespace mingw_thunk
