#pragma once

#include "internal/config.h"

#include "type_traits.h"

#include "internal/move_help.h"

namespace NS_NOSTL
{
  struct pair_first_construct_t
  {
  };
  constexpr pair_first_construct_t pair_first_construct{};

  template <typename T1, typename T2>
  struct pair
  {
    using first_type = T1;
    using second_type = T2;
    using this_type = pair<T1, T2>;

    T1 first;
    T2 second;

    template <typename TT1 = T1,
              typename TT2 = T2,
              typename = enable_if_t<is_default_constructible_v<TT1> &&
                                     is_default_constructible_v<TT2>>>
    constexpr pair() : first{}, second{}
    {
    }

    template <typename TT1 = T1,
              typename TT2 = T2,
              typename = enable_if_t<is_default_constructible_v<TT2>>>
    constexpr pair(pair_first_construct_t, const TT1 &x) : first{x}, second{}
    {
    }

    template <typename TT1 = T1,
              typename TT2 = T2,
              typename = enable_if_t<is_default_constructible_v<TT2>>>
    constexpr pair(pair_first_construct_t, TT1 &&x) : first{move(x)}, second{}
    {
    }

    template <typename TT1 = T1,
              typename TT2 = T2,
              typename = enable_if_t<is_copy_constructible_v<TT1> &&
                                     is_copy_constructible_v<TT2>>>
    constexpr pair(const T1 &x, const T2 &y) : first{x}, second{y}
    {
    }

    constexpr pair(const pair &) = default;
    constexpr pair(pair &&) = default;
    constexpr pair &operator=(const pair &) = default;
    constexpr pair &operator=(pair &&) = default;

    template <typename U,
              typename V,
              typename = enable_if_t<is_convertible_v<const U &, T1> &&
                                     is_convertible_v<const V &, T2>>>
    constexpr pair(const pair<U, V> &p) : first{p.first}, second{p.second}
    {
    }

    template <typename U,
              typename V,
              typename = enable_if_t<is_convertible_v<U, T1> &&
                                     is_convertible_v<V, T2>>>
    constexpr pair(U &&u, V &&v) : first(forward<U>(u)), second(forward<V>(v))
    {
    }

    template <typename U, typename = enable_if_t<is_convertible_v<U, T1>>>
    constexpr pair(U &&x, const T2 &y) : first(forward<U>(x)), second(y)
    {
    }

    template <typename V, typename = enable_if_t<is_convertible_v<V, T2>>>
    constexpr pair(const T1 &x, V &&y) : first(x), second(forward<V>(y))
    {
    }

    template <typename U,
              typename V,
              typename = enable_if_t<is_convertible_v<U, T1> &&
                                     is_convertible_v<V, T2>>>
    constexpr pair(pair<U, V> &&p)
        : first(forward<U>(p.first)), second(forward<V>(p.second))
    {
    }

  public:
    template <typename U,
              typename V,
              typename = enable_if_t<is_convertible_v<U, T1> &&
                                     is_convertible_v<V, T2>>>
    pair &operator=(const pair<U, V> &p)
    {
      first = p.first;
      second = p.second;
      return *this;
    }

    template <typename U,
              typename V,
              typename = enable_if_t<is_convertible_v<U, T1> &&
                                     is_convertible_v<V, T2>>>
    pair &operator=(pair<U, V> &&p)
    {
      first = forward<U>(p.first);
      second = forward<V>(p.second);
      return *this;
    }

    void swap(pair &p) noexcept(noexcept(swap(first, p.first)) && noexcept(swap(second, p.second)))
    {
      iter_swap(&first, &p.first);
      iter_swap(&second, &p.second);
    }
  };
} // namespace NS_NOSTL
