#pragma once

#include <stddef.h>

#include "internal/config.h"

namespace NS_NOCRT
{
  inline void *memcpy(void *dest, const void *src, size_t n)
  {
    char *dest_ = static_cast<char *>(dest);
    const char *src_ = static_cast<const char *>(src);
    while (n--)
      *dest_++ = *src_++;
    return dest;
  }

  inline void *memmove(void *dest, const void *src, size_t n)
  {
    if (dest < src)
      return NS_NOCRT::memcpy(dest, src, n);
    else {
      char *dest_ = static_cast<char *>(dest);
      const char *src_ = static_cast<const char *>(src);
      while (n--)
        dest_[n] = src_[n];
      return dest;
    }
  }
} // namespace NS_NOCRT
