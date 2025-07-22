#pragma once

#include "config.h"

namespace NS_NOSTL
{
  //////////////////////
  // is_constructible //
  //////////////////////

  template <typename T, typename... Args>
  struct is_constructible : public bool_constant<__is_constructible(T, Args...)>
  {
  };

  //////////////////////////////
  // is_default_constructible //
  //////////////////////////////

  template <typename T>
  struct is_default_constructible : public is_constructible<T>
  {
  };

  template <typename T>
  constexpr bool is_default_constructible_v =
      is_default_constructible<T>::value;

  ///////////////////////////
  // is_copy_constructible //
  ///////////////////////////

  template <typename T>
  struct is_copy_constructible
      : public is_constructible<T, add_lvalue_reference_t<add_const_t<T>>>
  {
  };

  template <typename T>
  constexpr bool is_copy_constructible_v = is_copy_constructible<T>::value;
} // namespace NS_NOSTL
