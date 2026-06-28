#pragma once

#include <thunk/_common.h>
#include <thunk/stdlib.h>

#include <windows.h>

// after <windows.h>
#include <iphlpapi.h>

namespace mingw_thunk
{
  namespace i
  {
    inline MIB_IFTABLE *init_if_table() noexcept
    {
      static MIB_IFTABLE *if_table = nullptr;
      static bool lock = false;

      MIB_IFTABLE *result = __atomic_load_n(&if_table, __ATOMIC_ACQUIRE);
      if (result)
        return result;

      while (__atomic_test_and_set(&lock, __ATOMIC_ACQUIRE))
        ;

      result = if_table;
      if (!result) {
        DWORD size = 0;
        GetIfTable(nullptr, &size, false);
        result = (MIB_IFTABLE *)c::malloc(size);
        if (result) {
          if (GetIfTable(result, &size, false) != NO_ERROR) {
            c::free(result);
            result = nullptr;
          }
        } else {
          SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        }
        __atomic_store_n(&if_table, result, __ATOMIC_RELEASE);
      }

      __atomic_clear(&lock, __ATOMIC_RELEASE);
      return result;
    }
  } // namespace i
} // namespace mingw_thunk
