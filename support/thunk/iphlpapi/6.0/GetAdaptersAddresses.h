#pragma once

#include <windows.h>
#include <winsock2.h>

// after <windows.h>
#include <iphlpapi.h>

namespace mingw_thunk
{
  namespace f
  {
    ULONG WINAPI fallback_GetAdaptersAddresses(_In_ ULONG Family,
                                               _In_ ULONG Flags,
                                               _In_ PVOID Reserved,
                                               _Inout_ PIP_ADAPTER_ADDRESSES
                                                   AdapterAddresses,
                                               _Inout_ PULONG SizePointer);
  }
} // namespace mingw_thunk
