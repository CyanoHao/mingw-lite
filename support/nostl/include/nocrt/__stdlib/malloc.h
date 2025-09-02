#pragma once

#include <heapapi.h>

#include "../errno.h"

namespace NS_NOCRT
{
  inline void *malloc(size_t size)
  {
    HANDLE heap = GetProcessHeap();
    void *allocation = HeapAlloc(heap, 0, size);
    if (!allocation) {
      NOCRT_errno = ENOMEM;
      return nullptr;
    }
    return allocation;
  }

  inline void *realloc(void *ptr, size_t size)
  {
    HANDLE heap = GetProcessHeap();
    void *allocation = nullptr;
    if (ptr)
      allocation = HeapReAlloc(heap, 0, ptr, size);
    else
      allocation = HeapAlloc(heap, 0, size);
    if (!allocation) {
      NOCRT_errno = ENOMEM;
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
