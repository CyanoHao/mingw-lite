#pragma once

#ifdef _WIN32
  #ifdef NOSTL_NOCRT
    #include <nocrt/malloc.h>
  #else
    #include <malloc.h>
  #endif
#else
  #include <stdlib.h>
#endif

#include "internal/config.h"

namespace NS_NOSTL
{
  class allocator
  {
  public:
    allocator(const char * = nullptr)
    {
    }

    allocator(const allocator &)
    {
    }

    allocator(const allocator &, const char *)
    {
    }

    allocator &operator=(const allocator &)
    {
      return *this;
    }

    bool operator==(const allocator &)
    {
      return true;
    }

    bool operator!=(const allocator &)
    {
      return false;
    }

    void *allocate(size_t n)
    {
#ifdef _WIN32
      return NS_NOSTL_CRT::_aligned_malloc(n, 1);
#else
      return ::malloc(n);
#endif
    }

    void *allocate(size_t n, size_t alignment)
    {
#ifdef _WIN32
      return NS_NOSTL_CRT::_aligned_malloc(n, align);
#else
      return ::aligned_alloc(alignment, n);
#endif
    }

    void deallocate(void *p, size_t /*n*/)
    {
#ifdef _WIN32
      NS_NOSTL_CRT::_aligned_free(p);
#else
      ::free(p);
#endif
    }

    const char *get_name() const
    {
      return "allocator";
    }

    void set_name()
    {
    }
  };
} // namespace NS_NOSTL
