#include "DwmEnableBlurBehindWindow.h"

#include <thunk/_common.h>

#include <dwmapi.h>
#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(dwmapi,
                 8,
                 HRESULT,
                 WINAPI,
                 DwmEnableBlurBehindWindow,
                 _In_ HWND hWnd,
                 _In_ const DWM_BLURBEHIND *pBlurBehind)
  {
    __DISPATCH_THUNK_2(DwmEnableBlurBehindWindow,
                       const auto pfn = try_get_DwmEnableBlurBehindWindow(),
                       pfn,
                       &f::fallback_DwmEnableBlurBehindWindow);

    return dllimport_DwmEnableBlurBehindWindow(hWnd, pBlurBehind);
  }

  namespace f
  {
    HRESULT WINAPI fallback_DwmEnableBlurBehindWindow(
        _In_ HWND hWnd, _In_ const DWM_BLURBEHIND *pBlurBehind)
    {
      return 0x80263001; // DWM_E_COMPOSITIONDISABLED
    }
  } // namespace f
} // namespace mingw_thunk
