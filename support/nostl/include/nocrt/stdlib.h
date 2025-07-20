#pragma once

#include <stddef.h>
#include <windows.h>

#include "internal/config.h"

namespace NS_NOCRT
{
  inline void *malloc(size_t size)
  {
    return HeapAlloc(GetProcessHeap(), 0, size);
  }

  inline void free(void *ptr)
  {
    if (ptr)
      HeapFree(GetProcessHeap(), 0, ptr);
  }
} // namespace NS_NOCRT
