#pragma once

#include <stddef.h>

#include "internal/config.h"

#include "allocator.h"
#include "initializer_list.h"
#include "iterator.h"
#include "memory.h"
#include "type_traits.h"

namespace NS_NOSTL
{
  template <typename T, typename Allocator>
  struct VectorBase {
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
    VectorBase()
        : mpBegin(nullptr), mpEnd(nullptr), mpCapacity(nullptr), mAllocator()
    {
    }

    VectorBase(const allocator_type &allocator)
        : mpBegin(nullptr)
        , mpEnd(nullptr)
        , mpCapacity(nullptr)
        , mAllocator(allocator)
    {
    }

    VectorBase(size_type n, const allocator_type &allocator)
        : mAllocator(allocator)
    {
      mpBegin = DoAllocate(n);
      mpEnd = mpBegin;
      internalCapacityPtr() = mpBegin + n;
    }

    ~VectorBase()
    {
      if (mpBegin) {
        internalAllocator().deallocate(
            mpBegin, (internalCapacityPtr() - mpBegin) * sizeof(T));
      }
    }

    const allocator_type &get_allocator() const noexcept
    {
      return internalAllocator();
    }

    allocator_type &get_allocator() noexcept { return internalAllocator(); }

    void set_allocator(const allocator_type &allocator)
    {
      internalAllocator() = allocator;
    }

  protected:
    T *DoAllocate(size_type n)
    {
      if (n) [[likely]]
        return (T *)allocate_memory(
            internalAllocator(), n * sizeof(T), alignof(T));
      else
        return nullptr;
    }

    void DoFree(T *p, size_type n)
    {
      if (p) {
        internalAllocator().deallocate(p, n * sizeof(T));
      }
    }
  };

  template <typename T, typename Allocator = allocator>
  class vector : public VectorBase<T, Allocator> {
    typedef VectorBase<T, Allocator> base_type;
    typedef vector<T, Allocator> this_type;

    template <class T2, class Allocator2, class U>
    friend typename vector<T2, Allocator2>::size_type
    erase_unsorted(vector<T2, Allocator2> &c, const U &value);

    template <class T2, class Allocator2, class P>
    friend typename vector<T2, Allocator2>::size_type
    erase_unsorted_if(vector<T2, Allocator2> &c, P predicate);

  protected:
    using base_type::mpBegin;
    using base_type::mpEnd;
    using base_type::mCapacityAllocator;
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

  public:
    vector() noexcept(noexcept(allocator_type())) : base_type() {}

    explicit vector(const allocator_type &allocator) noexcept
        : base_type(allocator)
    {
    }

    explicit vector(size_type n,
                    const allocator_type &allocator = allocator_type())
        : base_type(n, allocator)
    {
      uninitialized_value_construct_n(mpBegin, n);
      mpEnd = mpBegin + n;
    }

    vector(size_type n,
           const value_type &value,
           const allocator_type &allocator = allocator_type())
        : base_type(n, allocator)
    {
      uninitialized_fill_n(mpBegin, n, value);
      mpEnd = mpBegin + n;
    }

    vector(const this_type &x) : base_type(x.size(), x.internalAllocator())
    {
      mpEnd = uninitialized_copy(x.mpBegin, x.mpEnd, mpBegin);
    }

    vector(const this_type &x, const allocator_type &allocator)
        : base_type(x.size(), allocator)
    {
      mpEnd = uninitialized_copy(x.mpBegin, x.mpEnd, mpBegin);
    }

    vector(this_type &&x) noexcept : base_type(move(x.internalAllocator()))
    {
      DoSwap(x);
    }

    vector(this_type &&x, const allocator_type &allocator);
    vector(std::initializer_list<value_type> ilist,
           const allocator_type &allocator = allocator_type());

    // note: this has pre-C++11 semantics:
    // this constructor is equivalent to the constructor
    // vector(static_cast<size_type>(first), static_cast<value_type>(last),
    // allocator) if InputIterator is an integral type.
    template <typename InputIterator>
    vector(InputIterator first,
           InputIterator last,
           const allocator_type &allocator = allocator_type());

    ~vector();

    this_type &operator=(const this_type &x);
    this_type &operator=(std::initializer_list<value_type> ilist);
    this_type &operator=(
        this_type &&
            x); // TODO(c++17):
                // noexcept(allocator_traits<Allocator>::propagate_on_container_move_assignment::value
                // || allocator_traits<Allocator>::is_always_equal::value)

    void swap(
        this_type &
            x); // TODO(c++17):
                // noexcept(allocator_traits<Allocator>::propagate_on_container_move_assignment::value
                // || allocator_traits<Allocator>::is_always_equal::value)

    void assign(size_type n, const value_type &value);

    template <typename InputIterator>
    void assign(InputIterator first, InputIterator last);

    void assign(std::initializer_list<value_type> ilist);

    iterator begin() noexcept;
    const_iterator begin() const noexcept;
    const_iterator cbegin() const noexcept;

    iterator end() noexcept;
    const_iterator end() const noexcept;
    const_iterator cend() const noexcept;

    reverse_iterator rbegin() noexcept;
    const_reverse_iterator rbegin() const noexcept;
    const_reverse_iterator crbegin() const noexcept;

    reverse_iterator rend() noexcept;
    const_reverse_iterator rend() const noexcept;
    const_reverse_iterator crend() const noexcept;

    bool empty() const noexcept;
    size_type size() const noexcept;
    size_type capacity() const noexcept;

    void resize(size_type n, const value_type &value);
    void resize(size_type n);
    void reserve(size_type n);
    void set_capacity(
        size_type n =
            base_type::npos); // Revises the capacity to the user-specified
                              // value. Resizes the container to match the
                              // capacity if the requested capacity n is less
                              // than the current size. If n == npos then the
                              // capacity is reallocated (if necessary) such
                              // that capacity == size.
    void shrink_to_fit(); // C++11 function which is the same as set_capacity().

    pointer data() noexcept;
    const_pointer data() const noexcept;

    reference operator[](size_type n);
    const_reference operator[](size_type n) const;

    reference at(size_type n);
    const_reference at(size_type n) const;

    reference front();
    const_reference front() const;

    reference back();
    const_reference back() const;

    void push_back(const value_type &value);
    reference push_back();
    void *push_back_uninitialized();
    void push_back(value_type &&value);
    void pop_back();

    template <class... Args>
    iterator emplace(const_iterator position, Args &&...args);

    template <class... Args>
    reference emplace_back(Args &&...args);

    iterator insert(const_iterator position, const value_type &value);
    iterator
    insert(const_iterator position, size_type n, const value_type &value);
    iterator insert(const_iterator position, value_type &&value);
    iterator insert(const_iterator position,
                    std::initializer_list<value_type> ilist);

    // note: this has pre-C++11 semantics:
    // this function is equivalent to insert(const_iterator position,
    // static_cast<size_type>(first), static_cast<value_type>(last)) if
    // InputIterator is an integral type. ie. same as insert(const_iterator
    // position, size_type n, const value_type& value)
    template <typename InputIterator>
    iterator
    insert(const_iterator position, InputIterator first, InputIterator last);

    iterator erase_first(const T &value);
    iterator erase_first_unsorted(
        const T &value); // Same as erase, except it doesn't preserve order, but
                         // is faster because it simply copies the last item in
                         // the vector over the erased position.
    reverse_iterator erase_last(const T &value);
    reverse_iterator erase_last_unsorted(
        const T &value); // Same as erase, except it doesn't preserve order, but
                         // is faster because it simply copies the last item in
                         // the vector over the erased position.

    iterator erase(const_iterator position);
    iterator erase(const_iterator first, const_iterator last);
    iterator erase_unsorted(
        const_iterator
            position); // Same as erase, except it doesn't preserve order, but
                       // is faster because it simply copies the last item in
                       // the vector over the erased position.

    reverse_iterator erase(const_reverse_iterator position);
    reverse_iterator erase(const_reverse_iterator first,
                           const_reverse_iterator last);
    reverse_iterator erase_unsorted(const_reverse_iterator position);

    void clear() noexcept;
    void reset_lose_memory() noexcept; // This is a unilateral reset to an
                                       // initially empty state. No destructors
                                       // are called, no deallocation occurs.

    bool validate() const noexcept;
    int validate_iterator(const_iterator i) const noexcept;

  protected:
    // These functions do the real work of maintaining the vector. You will
    // notice that many of them have the same name but are specialized on
    // iterator_tag (iterator categories). This is because in these cases there
    // is an optimized implementation that can be had for some cases relative to
    // others. Functions which aren't referenced are neither compiled nor linked
    // into the application.
    template <bool bMove>
    struct should_move_or_copy_tag {};
    using should_copy_tag = should_move_or_copy_tag<false>;
    using should_move_tag = should_move_or_copy_tag<true>;

    template <typename ForwardIterator> // Allocates a pointer of array count n
                                        // and copy-constructs it with
                                        // [first,last).
    pointer DoRealloc(size_type n,
                      ForwardIterator first,
                      ForwardIterator last,
                      should_copy_tag);

    template <typename ForwardIterator> // Allocates a pointer of array count n
                                        // and copy-constructs it with
                                        // [first,last).
    pointer DoRealloc(size_type n,
                      ForwardIterator first,
                      ForwardIterator last,
                      should_move_tag);

    template <typename Integer>
    void DoInit(Integer n, Integer value, true_type);

    template <typename InputIterator>
    void DoInit(InputIterator first, InputIterator last, false_type);

    template <typename InputIterator>
    void DoInitFromIterator(InputIterator first,
                            InputIterator last,
                            input_iterator_tag);

    template <typename ForwardIterator>
    void DoInitFromIterator(ForwardIterator first,
                            ForwardIterator last,
                            forward_iterator_tag);

    template <typename Integer, bool bMove>
    void DoAssign(Integer n, Integer value, true_type);

    template <typename InputIterator, bool bMove>
    void DoAssign(InputIterator first, InputIterator last, false_type);

    void DoAssignValues(size_type n, const value_type &value);

    template <typename InputIterator, bool bMove>
    void DoAssignFromIterator(InputIterator first,
                              InputIterator last,
                              input_iterator_tag);

    template <typename RandomAccessIterator, bool bMove>
    void DoAssignFromIterator(RandomAccessIterator first,
                              RandomAccessIterator last,
                              random_access_iterator_tag);

    template <typename Integer>
    void DoInsert(const_iterator position, Integer n, Integer value, true_type);

    template <typename InputIterator>
    void DoInsert(const_iterator position,
                  InputIterator first,
                  InputIterator last,
                  false_type);

    template <typename InputIterator>
    void DoInsertFromIterator(const_iterator position,
                              InputIterator first,
                              InputIterator last,
                              input_iterator_tag);

    template <typename BidirectionalIterator>
    void DoInsertFromIterator(const_iterator position,
                              BidirectionalIterator first,
                              BidirectionalIterator last,
                              bidirectional_iterator_tag);

    void DoInsertValues(const_iterator position,
                        size_type n,
                        const value_type &value);

    void DoInsertValuesEnd(size_type n); // Default constructs n values
    void DoInsertValuesEnd(size_type n, const value_type &value);

    template <typename... Args>
    void DoInsertValue(const_iterator position, Args &&...args);

    template <typename... Args>
    void DoInsertValueEnd(Args &&...args);

    void DoClearCapacity();

    void DoGrow(size_type n);

    void DoSwap(this_type &x);
  };
} // namespace NS_NOSTL
