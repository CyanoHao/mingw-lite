#pragma once

#ifdef NOSTL_NOCRT
#include <nocrt/stdlib.h>
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

    void *allocate(size_t n, int /*flags*/ = 0)
    {
      return NS_NOSTL_CRT::malloc(n);
    }

    const char *get_name() const
    {
      return "allocator";
    }

    void deallocate(void *p, size_t /*n*/)
    {
      NS_NOSTL_CRT::free(p);
    }

    void set_name()
    {
    }
  };
} // namespace NS_NOSTL
