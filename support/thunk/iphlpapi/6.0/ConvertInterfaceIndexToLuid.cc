#include "ConvertInterfaceIndexToLuid.h"

#include "_internal.h"

#include <iphlpapi.h>
#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(iphlpapi,
                 8,
                 NETIO_STATUS,
                 WINAPI,
                 ConvertInterfaceIndexToLuid,
                 _In_ NET_IFINDEX InterfaceIndex,
                 _Out_ PNET_LUID InterfaceLuid)
  {
    __DISPATCH_THUNK_2(ConvertInterfaceIndexToLuid,
                       const auto pfn = try_get_ConvertInterfaceIndexToLuid(),
                       pfn,
                       &f::fallback_ConvertInterfaceIndexToLuid);

    return dllimport_ConvertInterfaceIndexToLuid(InterfaceIndex, InterfaceLuid);
  }

  namespace f
  {
    NETIO_STATUS WINAPI fallback_ConvertInterfaceIndexToLuid(
        _In_ NET_IFINDEX InterfaceIndex, _Out_ PNET_LUID InterfaceLuid)
    {
      MIB_IFTABLE *table = i::init_if_table();
      if (!table)
        return GetLastError();

      for (DWORD i = 0; i < table->dwNumEntries; ++i) {
        auto *row = table->table + i;
        if (row->dwIndex == InterfaceIndex) {
          InterfaceLuid->Info.NetLuidIndex = row->dwIndex;
          InterfaceLuid->Info.IfType = row->dwType;
          return NO_ERROR;
        }
      }
      return ERROR_NOT_FOUND;
    }
  } // namespace f
} // namespace mingw_thunk
