#pragma once

#include <windows.h>

// after <windows.h>
#include <iphlpapi.h>

namespace mingw_thunk
{
  namespace f
  {
    NETIO_STATUS WINAPI fallback_ConvertInterfaceNameToLuidW(
        _In_ const WCHAR *InterfaceName, _Out_ PNET_LUID InterfaceLuid);
  }
} // namespace mingw_thunk
