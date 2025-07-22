#pragma once

#include <stddef.h>

#include "internal/config.h"

#include "allocator.h"
#include "type_traits.h"

namespace NS_NOSTL
{
  template <typename T, typename Allocator>
  struct VectorBase
  {
  public:
    using allocator_type = Allocator;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    static constexpr size_type npos = size_type(-1);
    static constexpr size_type kMaxSize = size_type(-2);

    size_type GetNewCapacity(size_type currentCapacity)
    {
      return (currentCapacity > 0) ? (2 * currentCapacity) : 1;
    }

  protected:
   T *mpBegin;
   T *mpEnd;
  };
} // namespace NS_NOSTL
