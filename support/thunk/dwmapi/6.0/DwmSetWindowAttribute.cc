#include "DwmSetWindowAttribute.h"

#include <thunk/_common.h>

#include <dwmapi.h>
#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(dwmapi,
                 16,
                 HRESULT,
                 WINAPI,
                 DwmSetWindowAttribute,
                 _In_ HWND hwnd,
                 _In_ DWORD dwAttribute,
                 _In_ LPCVOID pvAttribute,
                 _In_ DWORD cbAttribute)
  {
    __DISPATCH_THUNK_2(DwmSetWindowAttribute,
                       const auto pfn = try_get_DwmSetWindowAttribute(),
                       pfn,
                       &f::fallback_DwmSetWindowAttribute);

    return dllimport_DwmSetWindowAttribute(
        hwnd, dwAttribute, pvAttribute, cbAttribute);
  }

  namespace f
  {
    HRESULT WINAPI fallback_DwmSetWindowAttribute(_In_ HWND hwnd,
                                                  _In_ DWORD dwAttribute,
                                                  _In_ LPCVOID pvAttribute,
                                                  _In_ DWORD cbAttribute)
    {
      SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
      return E_NOTIMPL;
    }
  } // namespace f
} // namespace mingw_thunk
