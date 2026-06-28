#include "DwmGetWindowAttribute.h"

#include <thunk/_common.h>

#include <dwmapi.h>
#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(dwmapi,
                 16,
                 HRESULT,
                 WINAPI,
                 DwmGetWindowAttribute,
                 _In_ HWND hwnd,
                 _In_ DWORD dwAttribute,
                 _Out_ PVOID pvAttribute,
                 _In_ DWORD cbAttribute)
  {
    __DISPATCH_THUNK_2(DwmGetWindowAttribute,
                       const auto pfn = try_get_DwmGetWindowAttribute(),
                       pfn,
                       &f::fallback_DwmGetWindowAttribute);

    return dllimport_DwmGetWindowAttribute(
        hwnd, dwAttribute, pvAttribute, cbAttribute);
  }

  namespace f
  {
    HRESULT WINAPI fallback_DwmGetWindowAttribute(_In_ HWND hwnd,
                                                  _In_ DWORD dwAttribute,
                                                  _Out_ PVOID pvAttribute,
                                                  _In_ DWORD cbAttribute)
    {
      SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
      return E_NOTIMPL;
    }
  } // namespace f
} // namespace mingw_thunk
