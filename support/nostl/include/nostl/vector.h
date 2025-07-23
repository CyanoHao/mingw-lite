#pragma once

#include <stddef.h>

#include "internal/config.h"

#include "allocator.h"
#include "iterator.h"
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
    T *mpCapacity;
    allocator_type mAllocator;

    T *&internalCapacityPtr() noexcept { return mpCapacity; }
    T *const &internalCapacityPtr() const noexcept { return mpCapacity; }
    allocator_type &internalAllocator() noexcept { return mAllocator; }
    const allocator_type &internalAllocator() const noexcept
    {
      return mAllocator;
    }

  public:
    VectorBase();
    VectorBase(const allocator_type &allocator);
    VectorBase(size_type n, const allocator_type &allocator);

    ~VectorBase();

    const allocator_type &get_allocator() const noexcept;
    allocator_type &get_allocator() noexcept;
    void set_allocator(const allocator_type &allocator);

  protected:
    T *DoAllocate(size_type n);
    void DoFree(T *p, size_type n);
  };

  template <typename T, typename Allocator = allocator>
  class vector : public VectorBase<T, Allocator>
  {
    using base_type = VectorBase<T, Allocator>;
    using this_type = vector<T, Allocator>;

    template <typename T2, typename Allocator2, typename U>
    friend typename vector<T2, Allocator2>::size_type
    erase_unsorted(vector<T2, Allocator2> &c, const U &value);

    template <class T2, class Allocator2, class P>
    friend typename vector<T2, Allocator2>::size_type
    erase_unsorted_if(vector<T2, Allocator2> &c, P predicate);

  protected:
    using base_type::mpBegin;
    using base_type::mpEnd;
    using base_type::mpCapacity;
    using base_type::mAllocator;
    using base_type::DoAllocate;
    using base_type::DoFree;
    using base_type::internalCapacityPtr;
    using base_type::internalAllocator;

  public:
    using value_type = T;
    using pointer = T *;
    using const_pointer = const T *;
    using reference = T &;
    using const_reference = const T &;
    using iterator = T *;
    using const_iterator = const T *;
    using reverse_iterator = NS_NOSTL::reverse_iterator<iterator>;
    using const_reverse_iterator = NS_NOSTL::reverse_iterator<const_iterator>;
    using size_type = typename base_type::size_type;
    using difference_type = typename base_type::difference_type;
    using allocator_type = typename base_type::allocator_type;

    using base_type::npos;
    using base_type::GetNewCapacity;

    static_assert(!is_const<value_type>::value,
                  "vector<T> value_type must be non-const.");
    static_assert(!is_volatile<value_type>::value,
                  "vector<T> value_type must be non-volatile.");
  };
} // namespace NS_NOSTL
