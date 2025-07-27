#pragma once

#include "config.h"

namespace NS_NOSTL
{
  template <typename T>
  constexpr T *addressof(T &value) noexcept
  {
    return __builtin_addressof(value);
  }
} // namespace NS_NOSTL
