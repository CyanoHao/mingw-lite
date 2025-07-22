#pragma once

#include "config.h"

namespace NS_NOSTL
{
  /////////////////////////
  // is_lvalue_reference //
  /////////////////////////

  template <typename T>
  struct is_lvalue_reference : false_type
  {
  };

  template <typename T>
  struct is_lvalue_reference<T &> : true_type
  {
  };

  template <typename T>
  constexpr bool is_lvalue_reference_v = is_lvalue_reference<T>::value;
} // namespace NS_NOSTL
