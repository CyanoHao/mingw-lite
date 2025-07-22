#pragma once

#include <stddef.h>
#include <windows.h>

#include "internal/config.h"

namespace NS_NOCRT
{
  inline void *malloc(size_t size)
  {
    HANDLE heap = GetProcessHeap();
    void *allocation = HeapAlloc(heap, 0, size);
    if (!allocation) {
      NS_NOCRT::errno = ENOMEM;
      return nullptr;
    }
    return allocation;
  }

  inline void free(void *ptr)
  {
    if (ptr) {
      HANDLE heap = GetProcessHeap();
      HeapFree(GetProcessHeap(), 0, ptr);
    }
  }
} // namespace NS_NOCRT
