#pragma once

#include "config.h"

namespace NS_NOSTL
{
  ////////////////////
  // is_convertible //
  ////////////////////

  template <typename From, typename To>
  struct is_convertible
      : public integral_constant<bool, __is_convertible_to(From, To)>
  {
  };

  template <typename From, typename To>
  constexpr bool is_convertible_v = is_convertible<From, To>::value;
} // namespace NS_NOSTL
