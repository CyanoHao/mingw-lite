#pragma once

#include <windows.h>

// after <windows.h>
#include <iphlpapi.h>

namespace mingw_thunk
{
  namespace f
  {
    NETIO_STATUS WINAPI
    fallback_ConvertInterfaceLuidToNameW(_In_ const NET_LUID *InterfaceLuid,
                                         _Out_ PWSTR InterfaceName,
                                         _In_ SIZE_T Length);
  }
} // namespace mingw_thunk
