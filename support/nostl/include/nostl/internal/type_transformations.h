#pragma once

#include "config.h"

namespace NS_NOSTL
{
  ///////////////
  // add_const //
  ///////////////
  template <typename T>
  struct add_const
  {
    using type = const T;
  };

  template <typename T>
  using add_const_t = typename add_const<T>::type;
} // namespace NS_NOSTL
