#include "DwmIsCompositionEnabled.h"

#include <thunk/_common.h>

#include <dwmapi.h>
#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(dwmapi,
                 4,
                 HRESULT,
                 WINAPI,
                 DwmIsCompositionEnabled,
                 _Out_ BOOL *pfEnabled)
  {
    __DISPATCH_THUNK_2(DwmIsCompositionEnabled,
                       const auto pfn = try_get_DwmIsCompositionEnabled(),
                       pfn,
                       &f::fallback_DwmIsCompositionEnabled);

    return dllimport_DwmIsCompositionEnabled(pfEnabled);
  }

  namespace f
  {
    HRESULT WINAPI fallback_DwmIsCompositionEnabled(_Out_ BOOL *pfEnabled)
    {
      if (pfEnabled) {
        *pfEnabled = FALSE;
        return S_OK;
      } else {
        SetLastError(ERROR_INVALID_PARAMETER);
        return E_INVALIDARG;
      }
    }
  } // namespace f
} // namespace mingw_thunk
