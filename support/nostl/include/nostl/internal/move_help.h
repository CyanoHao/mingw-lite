#pragma once

#include "config.h"

#include "../type_traits.h"

namespace NS_NOSTL
{
  template <typename T>
  constexpr T &&forward(remove_reference_t<T> &x) noexcept
  {
    return static_cast<T &&>(x);
  }

  template <typename T>
  constexpr T &&forward(remove_reference_t<T> &&x) noexcept
  {
    static_assert(!is_lvalue_reference_v<T>,
                  "cannot forward an rvalue as an lvalue");
    return static_cast<T &&>(x);
  }

  template <typename T>
  constexpr remove_reference_t<T> &&move(T &&x) noexcept
  {
    return static_cast<remove_reference_t<T> &&>(x);
  }
} // namespace NS_NOSTL
