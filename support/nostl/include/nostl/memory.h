#pragma once

#include "internal/config.h"

#include "iterator.h"
#include "type_traits.h"

#include "internal/copy_help.h"
#include "internal/memory_base.h"

namespace NS_NOSTL
{
  ////////////////////////
  // uninitialized_copy //
  ////////////////////////

  namespace internal
  {
    template <bool isTriviallyCopyable,
              bool isInputIteratorReferenceAddressable,
              bool areIteratorsContiguous>
    struct uninitialized_copy_impl {
      template <typename InputIterator, typename ForwardIterator>
      static ForwardIterator
      impl(InputIterator first, InputIterator last, ForwardIterator dest)
      {
        using value_type =
            typename iterator_traits<ForwardIterator>::value_type;
        ForwardIterator currentDest(dest);
        for (; first != last; ++first, ++currentDest)
          ::new (static_cast<void *>(__builtin_addressof(*currentDest)))
              value_type(*first);
        return currentDest;
      }
    };

    template <>
    struct uninitialized_copy_impl<true, true, false> {
      template <typename InputIterator, typename ForwardIterator>
      static ForwardIterator
      impl(InputIterator first, InputIterator last, ForwardIterator dest)
      {
        using value_type =
            typename iterator_traits<ForwardIterator>::value_type;
        for (; first != last; ++first, ++dest)
          NS_NOSTL_CRT::memmove(__builtin_addressof(*dest),
                                __builtin_addressof(*first),
                                sizeof(value_type));

        return dest;
      }
    };

    template <>
    struct uninitialized_copy_impl<true, true, true> {
      template <typename InputIterator, typename ForwardIterator>
      static ForwardIterator
      impl(InputIterator first, InputIterator last, ForwardIterator dest)
      {
        using value_type =
            typename iterator_traits<ForwardIterator>::value_type;
        if (first == last) [[unlikely]]
          return dest;
        auto count = (last - first);
        NS_NOSTL_CRT::memmove(__builtin_addressof(*dest),
                              __builtin_addressof(*first),
                              sizeof(value_type) * count);
        return dest + count;
      }
    };
  } // namespace internal

  template <typename InputIterator, typename ForwardIterator>
  inline ForwardIterator uninitialized_copy(InputIterator first,
                                            InputIterator last,
                                            ForwardIterator result)
  {
    using IIC = typename iterator_traits<InputIterator>::iterator_category;
    using OIC = typename iterator_traits<ForwardIterator>::iterator_category;
    using value_type_input =
        typename iterator_traits<InputIterator>::value_type;
    using value_type_output =
        typename iterator_traits<ForwardIterator>::value_type;

    constexpr bool isTriviallyCopyable =
        is_same_v<value_type_input, value_type_output> &&
        is_trivially_copyable_v<value_type_output>;

    constexpr bool isInputIteratorReferenceAddressable =
        is_convertible_v<typename add_lvalue_reference<value_type_input>::type,
                         typename iterator_traits<InputIterator>::reference>;

    constexpr bool areIteratorsContiguous =
        (is_pointer_v<InputIterator> ||
         internal::is_contiguous_iterator_v<IIC>) &&
        (is_pointer_v<ForwardIterator> ||
         internal::is_contiguous_iterator_v<OIC>);

    return internal::uninitialized_copy_impl<
        isTriviallyCopyable,
        isInputIteratorReferenceAddressable,
        areIteratorsContiguous>::impl(first, last, result);
  }

  /////////////////////////////////////
  // uninitialized_value_construct_n //
  /////////////////////////////////////

  template <class ForwardIterator, class Count>
  ForwardIterator uninitialized_value_construct_n(ForwardIterator first,
                                                  Count n)
  {
    using value_type =
        typename NS_NOSTL::iterator_traits<ForwardIterator>::value_type;
    ForwardIterator currentDest(first);

    for (; n > 0; --n, ++currentDest)
      ::new (__builtin_addressof(*currentDest)) value_type();
    return currentDest;
  }

  //////////////////////////
  // uninitialized_fill_n //
  //////////////////////////

  namespace internal
  {
    template <typename ForwardIterator, typename Count, typename T>
    inline void uninitialized_fill_n_impl(ForwardIterator first,
                                          Count n,
                                          const T &value,
                                          true_type)
    {
      fill_n(first, n, value);
    }

    template <typename ForwardIterator, typename Count, typename T>
    void uninitialized_fill_n_impl(ForwardIterator first,
                                   Count n,
                                   const T &value,
                                   false_type)
    {
      using value_type = typename iterator_traits<ForwardIterator>::value_type;
      ForwardIterator currentDest(first);
      for (; n > 0; --n, ++currentDest)
        ::new ((void *)addressof(*currentDest)) value_type(value);
    }
  } // namespace internal

  template <typename ForwardIterator, typename Count, typename T>
  inline void
  uninitialized_fill_n(ForwardIterator first, Count n, const T &value)
  {
    using value_type = typename iterator_traits<ForwardIterator>::value_type;
    internal::uninitialized_fill_n_impl(
        first, n, value, is_trivially_copy_assignable<value_type>());
  }

} // namespace NS_NOSTL
