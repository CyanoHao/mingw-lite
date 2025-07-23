#pragma once

#include <stddef.h>

#include "internal/config.h"

#include "internal/type_void_t.h"

namespace NS_NOSTL
{
  struct input_iterator_tag
  {
  };
  struct output_iterator_tag
  {
  };
  struct forward_iterator_tag : public input_iterator_tag
  {
  };
  struct bidirectional_iterator_tag : public forward_iterator_tag
  {
  };
  struct random_access_iterator_tag : public bidirectional_iterator_tag
  {
  };
  struct contiguous_iterator_tag : public random_access_iterator_tag
  {
  };

  namespace internal
  {
    template <typename Iterator, typename = void>
    struct default_iterator_traits
    {
    };

    template <typename Iterator>
    struct default_iterator_traits<Iterator,
                                   void_t<typename Iterator::iterator_category,
                                          typename Iterator::value_type,
                                          typename Iterator::difference_type,
                                          typename Iterator::pointer,
                                          typename Iterator::reference>>
    {
      using iterator_category = typename Iterator::iterator_category;
      using value_type = typename Iterator::value_type;
      using difference_type = typename Iterator::difference_type;
      using pointer = typename Iterator::pointer;
      using reference = typename Iterator::reference;
    };
  } // namespace internal

  template <typename Iterator>
  struct iterator_traits : internal::default_iterator_traits<Iterator>
  {
  };

  template <typename T>
  struct iterator_traits<T *>
  {
    using iterator_category = contiguous_iterator_tag;
    using value_type = T;
    using difference_type = ptrdiff_t;
    using pointer = T *;
    using reference = T &;
  };

  template <typename T>
  struct iterator_traits<const T *>
  {
    using iterator_category = contiguous_iterator_tag;
    using value_type = const T;
    using difference_type = ptrdiff_t;
    using pointer = const T *;
    using reference = const T &;
  };

  template <typename Iterator>
  class reverse_iterator
  {
  public:
    using iterator_type = Iterator;
    using iterator_category =
        typename iterator_traits<Iterator>::iterator_category;
    using value_type = typename iterator_traits<Iterator>::value_type;
    using difference_type = typename iterator_traits<Iterator>::difference_type;
    using pointer = typename iterator_traits<Iterator>::pointer;
    using reference = typename iterator_traits<Iterator>::reference;

  protected:
    Iterator mIterator;

  public:
    constexpr reverse_iterator() : mIterator() {}

    constexpr explicit reverse_iterator(iterator_type i) : mIterator(i) {}

    template <typename U>
    constexpr reverse_iterator(const reverse_iterator<U> &ri)
        : mIterator(ri.base())
    {
    }

    template <typename U>
    constexpr reverse_iterator<Iterator> &
    operator=(const reverse_iterator<U> &ri)
    {
      mIterator = ri.base();
      return *this;
    }

    constexpr iterator_type base() const { return mIterator; }

    constexpr reference operator*() const
    {
      iterator_type i(mIterator);
      return *--i;
    }

    constexpr pointer operator->() const { return &(operator*()); }

    constexpr reverse_iterator &operator++()
    {
      --mIterator;
      return *this;
    }

    constexpr reverse_iterator operator++(int)
    {
      reverse_iterator ri(*this);
      --mIterator;
      return ri;
    }

    constexpr reverse_iterator &operator--()
    {
      ++mIterator;
      return *this;
    }

    constexpr reverse_iterator operator--(int)
    {
      reverse_iterator ri(*this);
      ++mIterator;
      return ri;
    }

    constexpr reverse_iterator operator+(difference_type n) const
    {
      return reverse_iterator(mIterator - n);
    }

    constexpr reverse_iterator &operator+=(difference_type n)
    {
      mIterator -= n;
      return *this;
    }

    constexpr reverse_iterator operator-(difference_type n) const
    {
      return reverse_iterator(mIterator + n);
    }

    constexpr reverse_iterator &operator-=(difference_type n)
    {
      mIterator += n;
      return *this;
    }

    constexpr reference operator[](difference_type n) const
    {
      return mIterator[-n - 1];
    }
  };

  template <typename Iterator1, typename Iterator2>
  constexpr inline bool operator==(const reverse_iterator<Iterator1> &a,
                                   const reverse_iterator<Iterator2> &b)
  {
    return a.base() == b.base();
  }

  template <typename Iterator1, typename Iterator2>
  constexpr inline bool operator<(const reverse_iterator<Iterator1> &a,
                                  const reverse_iterator<Iterator2> &b)
  {
    return a.base() > b.base();
  }

  template <typename Iterator1, typename Iterator2>
  constexpr inline bool operator!=(const reverse_iterator<Iterator1> &a,
                                   const reverse_iterator<Iterator2> &b)
  {
    return a.base() != b.base();
  }

  template <typename Iterator1, typename Iterator2>
  constexpr inline bool operator>(const reverse_iterator<Iterator1> &a,
                                  const reverse_iterator<Iterator2> &b)
  {
    return a.base() < b.base();
  }

  template <typename Iterator1, typename Iterator2>
  constexpr inline bool operator<=(const reverse_iterator<Iterator1> &a,
                                   const reverse_iterator<Iterator2> &b)
  {
    return a.base() >= b.base();
  }

  template <typename Iterator1, typename Iterator2>
  constexpr inline bool operator>=(const reverse_iterator<Iterator1> &a,
                                   const reverse_iterator<Iterator2> &b)
  {
    return a.base() <= b.base();
  }

  template <typename Iterator1, typename Iterator2>
  constexpr inline typename reverse_iterator<Iterator1>::difference_type
  operator-(const reverse_iterator<Iterator1> &a,
            const reverse_iterator<Iterator2> &b)
  {
    return b.base() - a.base();
  }

  template <typename Iterator>
  constexpr inline reverse_iterator<Iterator>
  operator+(typename reverse_iterator<Iterator>::difference_type n,
            const reverse_iterator<Iterator> &a)
  {
    return reverse_iterator<Iterator>(a.base() - n);
  }
} // namespace NS_NOSTL
