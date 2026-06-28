#include "ConvertInterfaceLuidToIndex.h"

#include "_internal.h"

#include <iphlpapi.h>
#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(iphlpapi,
                 8,
                 NETIO_STATUS,
                 WINAPI,
                 ConvertInterfaceLuidToIndex,
                 _In_ const NET_LUID *InterfaceLuid,
                 _Out_ PNET_IFINDEX InterfaceIndex)
  {
    __DISPATCH_THUNK_2(ConvertInterfaceLuidToIndex,
                       const auto pfn = try_get_ConvertInterfaceLuidToIndex(),
                       pfn,
                       &f::fallback_ConvertInterfaceLuidToIndex);

    return dllimport_ConvertInterfaceLuidToIndex(InterfaceLuid, InterfaceIndex);
  }

  namespace f
  {
    NETIO_STATUS WINAPI fallback_ConvertInterfaceLuidToIndex(
        _In_ const NET_LUID *InterfaceLuid, _Out_ PNET_IFINDEX InterfaceIndex)
    {
      MIB_IFTABLE *table = i::init_if_table();
      if (!table)
        return GetLastError();

      for (DWORD i = 0; i < table->dwNumEntries; ++i) {
        auto *row = table->table + i;
        if (row->dwIndex == InterfaceLuid->Info.NetLuidIndex &&
            row->dwType == InterfaceLuid->Info.IfType) {
          *InterfaceIndex = row->dwIndex;
          return NO_ERROR;
        }
      }
      return ERROR_NOT_FOUND;
    }
  } // namespace f
} // namespace mingw_thunk
