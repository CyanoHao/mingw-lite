#pragma once

#include <windows.h>

// after <windows.h>
#include <iphlpapi.h>

namespace mingw_thunk
{
  namespace f
  {
    NETIO_STATUS WINAPI fallback_ConvertInterfaceLuidToIndex(
        _In_ const NET_LUID *InterfaceLuid, _Out_ PNET_IFINDEX InterfaceIndex);
  }
} // namespace mingw_thunk
