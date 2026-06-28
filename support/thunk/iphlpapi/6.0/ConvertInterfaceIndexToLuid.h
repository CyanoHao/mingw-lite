#pragma once

#include <windows.h>

// after <windows.h>
#include <iphlpapi.h>

namespace mingw_thunk
{
  namespace f
  {
    NETIO_STATUS WINAPI fallback_ConvertInterfaceIndexToLuid(
        _In_ NET_IFINDEX InterfaceIndex, _Out_ PNET_LUID InterfaceLuid);
  }
} // namespace mingw_thunk
