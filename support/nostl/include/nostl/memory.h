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

  template <typename InputIterator, typename ForwardIterator>
  inline ForwardIterator uninitialized_copy(InputIterator first,
                                            InputIterator last,
                                            ForwardIterator result)
  {
    using value_type = typename iterator_traits<ForwardIterator>::value_type;
    ForwardIterator currentDest(result);
    for (; first != last; ++first, ++currentDest)
      ::new (__builtin_addressof(*currentDest)) value_type(*first);
    return currentDest;
  }

  /////////////////////////////////////
  // uninitialized_value_construct_n //
  /////////////////////////////////////

  template <class ForwardIterator, class Count>
  ForwardIterator uninitialized_value_construct_n(ForwardIterator first,
                                                  Count n)
  {
    using value_type = typename iterator_traits<ForwardIterator>::value_type;
    ForwardIterator currentDest(first);
    for (; n > 0; --n, ++currentDest)
      ::new (__builtin_addressof(*currentDest)) value_type();
    return currentDest;
  }

  //////////////////////////
  // uninitialized_fill_n //
  //////////////////////////

  template <typename ForwardIterator, typename Count, typename T>
  inline void
  uninitialized_fill_n(ForwardIterator first, Count n, const T &value)
  {
    using value_type = typename iterator_traits<ForwardIterator>::value_type;
    ForwardIterator currentDest(first);
    for (; n > 0; --n, ++currentDest)
      ::new (__builtin_addressof(*currentDest)) value_type(value);
  }

  //////////////
  // destruct //
  //////////////

  template <typename T>
  inline void destruct(T *p)
  {
    p->~T();
  }

  template <typename ForwardIterator>
  inline void destruct(ForwardIterator first, ForwardIterator last)
  {
    using value_type = typename iterator_traits<ForwardIterator>::value_type;
    for (; first != last; ++first)
      (*first).~value_type();
  }

} // namespace NS_NOSTL
