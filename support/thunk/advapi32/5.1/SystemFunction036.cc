#include "SystemFunction036.h"

#include <thunk/_common.h>
#include <thunk/rand.h>
#include <thunk/string.h>

#include <stdint.h>

#include <ntsecapi.h>
#include <windows.h>

namespace mingw_thunk
{
  // documented as RtlGenRandom
  __DEFINE_THUNK(advapi32,
                 8,
                 BOOLEAN,
                 WINAPI,
                 SystemFunction036,
                 _Out_ PVOID RandomBuffer,
                 _In_ ULONG RandomBufferLength)
  {
    __DISPATCH_THUNK_2(SystemFunction036,
                       const auto pfn = try_get_SystemFunction036(),
                       pfn,
                       &f::fallback_SystemFunction036);

    return dllimport_SystemFunction036(RandomBuffer, RandomBufferLength);
  }

  namespace f
  {
    BOOLEAN WINAPI fallback_SystemFunction036(_Out_ PVOID RandomBuffer,
                                              _In_ ULONG RandomBufferLength)
    {
      if (!RandomBuffer)
        return FALSE;

      // Unaligned buffer is rare, here we simply ignore it.

      constexpr size_t block_size = sizeof(uint32_t);
      const size_t blocks = RandomBufferLength / block_size;
      const size_t remainder = RandomBufferLength % block_size;

      uint32_t *buffer = reinterpret_cast<uint32_t *>(RandomBuffer);

      for (size_t i = 0; i < blocks; ++i)
        buffer[i] = i::rand32();

      if (remainder) {
        uint32_t value = i::rand32();
        c::memcpy(buffer + blocks, &value, remainder);
      }

      return TRUE;
    }
  } // namespace f
} // namespace mingw_thunk
