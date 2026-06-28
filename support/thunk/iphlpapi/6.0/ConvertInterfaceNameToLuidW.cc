#include "ConvertInterfaceNameToLuidW.h"

#include "_internal.h"

#include <thunk/string.h>

#include <iphlpapi.h>
#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(iphlpapi,
                 8,
                 NETIO_STATUS,
                 WINAPI,
                 ConvertInterfaceNameToLuidW,
                 _In_ const WCHAR *InterfaceName,
                 _Out_ PNET_LUID InterfaceLuid)
  {
    __DISPATCH_THUNK_2(ConvertInterfaceNameToLuidW,
                       const auto pfn = try_get_ConvertInterfaceNameToLuidW(),
                       pfn,
                       &f::fallback_ConvertInterfaceNameToLuidW);

    return dllimport_ConvertInterfaceNameToLuidW(InterfaceName, InterfaceLuid);
  }

  namespace f
  {
    NETIO_STATUS WINAPI fallback_ConvertInterfaceNameToLuidW(
        _In_ const WCHAR *InterfaceName, _Out_ PNET_LUID InterfaceLuid)
    {
      MIB_IFTABLE *table = i::init_if_table();
      if (!table)
        return GetLastError();

      for (DWORD i = 0; i < table->dwNumEntries; ++i) {
        auto *row = table->table + i;
        if (c::wcscmp(row->wszName, InterfaceName) == 0) {
          InterfaceLuid->Info.NetLuidIndex = row->dwIndex;
          InterfaceLuid->Info.IfType = row->dwType;
          return NO_ERROR;
        }
      }
      return ERROR_NOT_FOUND;
    }
  } // namespace f
} // namespace mingw_thunk
