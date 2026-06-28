#include "GetAdaptersAddresses.h"

#include <thunk/_common.h>
#include <thunk/os.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(iphlpapi,
                 20,
                 ULONG,
                 WINAPI,
                 GetAdaptersAddresses,
                 _In_ ULONG Family,
                 _In_ ULONG Flags,
                 _In_ PVOID Reserved,
                 _Inout_ PIP_ADAPTER_ADDRESSES AdapterAddresses,
                 _Inout_ PULONG SizePointer)
  {
    const auto pfn = try_get_GetAdaptersAddresses();
    __DISPATCH_THUNK_2(GetAdaptersAddresses,
                       pfn && i::os_version() >= g::win32_vista,
                       pfn,
                       &f::fallback_GetAdaptersAddresses);

    return dllimport_GetAdaptersAddresses(
        Family, Flags, Reserved, AdapterAddresses, SizePointer);
  }

  namespace f
  {
    ULONG WINAPI fallback_GetAdaptersAddresses(_In_ ULONG Family,
                                               _In_ ULONG Flags,
                                               _In_ PVOID Reserved,
                                               _Inout_ PIP_ADAPTER_ADDRESSES
                                                   AdapterAddresses,
                                               _Inout_ PULONG SizePointer)
    {
      // behaviour changed in Windows Vista; XP result is not used
      SetLastError(ERROR_NOT_SUPPORTED);
      return ERROR_NOT_SUPPORTED;
    }
  } // namespace f
} // namespace mingw_thunk
