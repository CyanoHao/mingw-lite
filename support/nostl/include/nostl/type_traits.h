#pragma once

#include "internal/config.h"

namespace NS_NOSTL
{

  ///////////////////////
  // integral_constant //
  ///////////////////////

  template <typename T, T v>
  struct integral_constant
  {
    static constexpr T value = v;
    using value_type = T;
    using type = integral_constant<T, v>;

    constexpr operator value_type() const noexcept { return value; }
    constexpr value_type operator()() const noexcept { return value; }
  };

  ///////////////////////////
  // true_type, false_type //
  ///////////////////////////

  using true_type = integral_constant<bool, true>;
  using false_type = integral_constant<bool, false>;

  ///////////////////
  // bool_constant //
  ///////////////////

  template <bool B>
  using bool_constant = integral_constant<bool, B>;

  ///////////////////////////
  // enable_if, disable_if //
  ///////////////////////////

  template <bool B, typename T = void>
  struct enable_if
  {
  };

  template <typename T>
  struct enable_if<true, T>
  {
    using type = T;
  };

  template <bool B, typename T = void>
  using enable_if_t = typename enable_if<B, T>::type;

  template <bool B, typename T>
  struct disable_if
  {
  };

  template <typename T>
  struct disable_if<false, T>
  {
    using type = T;
  };

  template <bool B, typename T>
  using disable_if_t = typename disable_if<B, T>::type;

  ///////////////////
  // type_identity //
  ///////////////////

  template <typename T>
  struct type_identity
  {
    using type = T;
  };

  template <typename T>
  using type_identity_t = typename type_identity<T>::type;

  //////////////////////
  // remove_reference //
  //////////////////////

  template <typename T>
  struct remove_reference
  {
    using type = T;
  };

  template <typename T>
  struct remove_reference<T &>
  {
    using type = T;
  };

  template <typename T>
  struct remove_reference<T &&>
  {
    using type = T;
  };

  template <typename T>
  using remove_reference_t = typename remove_reference<T>::type;

  //////////////////////////
  // add_lvalue_reference //
  //////////////////////////

  namespace internal
  {
    template <typename T>
    auto try_add_lvalue_reference(int) -> type_identity<T &>;

    template <typename T>
    auto try_add_lvalue_reference(...) -> type_identity<T>;
  } // namespace internal

  template <typename T>
  struct add_lvalue_reference
      : decltype(internal::try_add_lvalue_reference<T>(0))
  {
  };

  template <typename T>
  using add_lvalue_reference_t = typename add_lvalue_reference<T>::type;
} // namespace NS_NOSTL

#include "internal/type_transformations.h"

#include "internal/type_properties.h"

#include "internal/type_compound.h"

#include "internal/type_pod.h"
