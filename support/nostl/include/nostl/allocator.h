#pragma once

#include <stddef.h>

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
  class allocator {
  public:
    allocator(const char * = nullptr) {}

    allocator(const allocator &) {}

    allocator(const allocator &, const char *) {}

    allocator &operator=(const allocator &) { return *this; }

    bool operator==(const allocator &) { return true; }

    bool operator!=(const allocator &) { return false; }

    void *allocate(size_t n)
    {
#ifdef _WIN32
      void *result = NS_NOSTL_CRT::_aligned_malloc(n, 1);
#else
      void *result = ::malloc(n);
#endif
      if (!result)
        NOSTL_RAISE_BAD_ALLOC();
      return result;
    }

    void *allocate(size_t n, size_t alignment)
    {
#ifdef _WIN32
      void *result = NS_NOSTL_CRT::_aligned_malloc(n, alignment);
#else
      void *result = ::aligned_alloc(alignment, n);
#endif
      if (!result)
        NOSTL_RAISE_BAD_ALLOC();
      return result;
    }

    void deallocate(void *p, size_t /*n*/)
    {
#ifdef _WIN32
      NS_NOSTL_CRT::_aligned_free(p);
#else
      ::free(p);
#endif
    }

    const char *get_name() const { return "allocator_malloc"; }

    void set_name() {}
  };
} // namespace NS_NOSTL

namespace NS_NOSTL
{
  namespace internal
  {
#ifdef __cpp_inline_variables
    inline allocator gDefaultAllocator;
    inline allocator *gpDefaultAllocator = &gDefaultAllocator;
#else
    inline allocator &gDefaultAllocator()
    {
      static allocator allocator;
      return allocator;
    }
    inline allocator *&gpDefaultAllocator()
    {
      static allocator *pAllocator = &gDefaultAllocator();
      return pAllocator;
    }
#endif
  } // namespace internal

  inline allocator *GetDefaultAllocator()
  {

#ifdef __cpp_inline_variables
    return internal::gpDefaultAllocator;
#else
    return internal::gpDefaultAllocator();
#endif
  }

  inline allocator *SetDefaultAllocator(allocator *pAllocator)
  {
#ifdef __cpp_inline_variables
    allocator *const pPrevAllocator = internal::gpDefaultAllocator;
    internal::gpDefaultAllocator = pAllocator;
#else
    allocator *const pPrevAllocator = internal::gpDefaultAllocator();
    internal::gpDefaultAllocator() = pAllocator;
#endif
    return pPrevAllocator;
  }

  template <typename Allocator>
  inline void *allocate_memory(Allocator &a, size_t n, size_t alignment)
  {
    void *result = nullptr;

    // do not trust __STDCPP_DEFAULT_NEW_ALIGNMENT__, it may be wrong
    // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=90569
    if (alignment < alignof(max_align_t)) {
      result = a.allocate(n);
    } else {
      result = a.allocate(n, alignment);
    }
    return result;
  }
} // namespace NS_NOSTL
