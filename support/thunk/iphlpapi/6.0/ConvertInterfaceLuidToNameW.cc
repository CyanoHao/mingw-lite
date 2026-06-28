#include "ConvertInterfaceLuidToNameW.h"

#include "_internal.h"

#include <thunk/string.h>

#include <iphlpapi.h>
#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(iphlpapi,
                 12,
                 NETIO_STATUS,
                 WINAPI,
                 ConvertInterfaceLuidToNameW,
                 _In_ const NET_LUID *InterfaceLuid,
                 _Out_ PWSTR InterfaceName,
                 _In_ SIZE_T Length)
  {
    __DISPATCH_THUNK_2(ConvertInterfaceLuidToNameW,
                       const auto pfn = try_get_ConvertInterfaceLuidToNameW(),
                       pfn,
                       &f::fallback_ConvertInterfaceLuidToNameW);

    return dllimport_ConvertInterfaceLuidToNameW(
        InterfaceLuid, InterfaceName, Length);
  }

  namespace f
  {
    NETIO_STATUS WINAPI
    fallback_ConvertInterfaceLuidToNameW(_In_ const NET_LUID *InterfaceLuid,
                                         _Out_ PWSTR InterfaceName,
                                         _In_ SIZE_T Length)
    {
      MIB_IFTABLE *table = i::init_if_table();
      if (!table)
        return GetLastError();

      for (DWORD i = 0; i < table->dwNumEntries; ++i) {
        auto *row = table->table + i;
        if (row->dwIndex == InterfaceLuid->Info.NetLuidIndex &&
            row->dwType == InterfaceLuid->Info.IfType) {
          if (Length < c::wcslen(row->wszName) + 1)
            return ERROR_INSUFFICIENT_BUFFER;
          c::wcscpy(InterfaceName, row->wszName);
          return NO_ERROR;
        }
      }
      return ERROR_NOT_FOUND;
    }
  } // namespace f
} // namespace mingw_thunk
