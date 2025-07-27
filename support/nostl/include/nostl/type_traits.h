#pragma once

#include <stddef.h>

#include "internal/config.h"

namespace NS_NOSTL
{

  ///////////////////////
  // integral_constant //
  ///////////////////////

  template <typename T, T v>
  struct integral_constant {
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

  ///////////////////
  // argument_sink //
  ///////////////////

  struct argument_sink {
    template <typename... Args>
    argument_sink(Args &&...)
    {
    }
  };

  ///////////////////////////
  // enable_if, disable_if //
  ///////////////////////////

  template <bool B, typename T = void>
  struct enable_if {};
  template <typename T>
  struct enable_if<true, T> {
    using type = T;
  };

  template <bool B, typename T = void>
  using enable_if_t = typename enable_if<B, T>::type;

  template <bool B, typename T>
  struct disable_if {};
  template <typename T>
  struct disable_if<false, T> {
    using type = T;
  };

  template <bool B, typename T>
  using disable_if_t = typename disable_if<B, T>::type;

  /////////////////
  // conditional //
  /////////////////

  template <bool B, typename T, typename F>
  struct conditional {
    using type = T;
  };
  template <typename T, typename F>
  struct conditional<false, T, F> {
    using type = F;
  };

  template <bool B, typename T, typename F>
  using conditional_t = typename conditional<B, T, F>::type;

  /////////////////
  // conjunction //
  /////////////////

  template <typename... B>
  struct conjunction : true_type {};
  template <typename B>
  struct conjunction<B> : B {};
  template <typename B, typename... Bn>
  struct conjunction<B, Bn...>
      : conditional_t<bool(B::value), conjunction<Bn...>, B> {};

  template <typename... B>
  constexpr bool conjunction_v = conjunction<B...>::value;

  /////////////////
  // disjunction //
  /////////////////

  template <typename... B>
  struct disjunction : false_type {};
  template <typename B>
  struct disjunction<B> : B {};
  template <typename B, typename... Bn>
  struct disjunction<B, Bn...>
      : conditional_t<bool(B::value), B, disjunction<Bn...>> {};

  template <typename... B>
  constexpr bool disjunction_v = disjunction<B...>::value;

  //////////////
  // negation //
  //////////////

  template <typename B>
  struct negation : bool_constant<!bool(B::value)> {};

  template <typename B>
  constexpr bool negation_v = negation<B>::value;

  ///////////////////
  // type_identity //
  ///////////////////

  template <typename T>
  struct type_identity {
    using type = T;
  };

  template <typename T>
  using type_identity_t = typename type_identity<T>::type;

  /////////////
  // is_same //
  /////////////

  template <typename T, typename U>
  struct is_same : false_type {};
  template <typename T>
  struct is_same<T, T> : true_type {};

  template <typename T, typename U>
  constexpr bool is_same_v = is_same<T, U>::value;

  //////////////
  // is_const //
  //////////////

  template <typename T>
  struct is_const : false_type {};
  template <typename T>
  struct is_const<const T> : true_type {};

  template <typename T>
  constexpr bool is_const_v = is_const<T>::value;

  /////////////////
  // is_volatile //
  /////////////////

  template <typename T>
  struct is_volatile : false_type {};
  template <typename T>
  struct is_volatile<volatile T> : true_type {};

  template <typename T>
  constexpr bool is_volatile_v = is_volatile<T>::value;

  //////////////////
  // is_reference //
  ///////////////////

  template <typename T>
  struct is_reference : false_type {};
  template <typename T>
  struct is_reference<T &> : true_type {};
  template <typename T>
  struct is_reference<T &&> : true_type {};

  template <typename T>
  constexpr bool is_reference_v = is_reference<T>::value;

  /////////////////
  // is_function //
  /////////////////

  template <typename T>
  struct is_function
      : bool_constant<!is_reference_v<T> && !is_const_v<const T>> {};

  template <typename T>
  constexpr bool is_function_v = is_function<T>::value;

  //////////////////
  // remove_const //
  //////////////////

  template <typename T>
  struct remove_const {
    using type = T;
  };
  template <typename T>
  struct remove_const<const T> {
    using type = T;
  };
  template <typename T>
  struct remove_const<const T[]> {
    using type = T[];
  };
  template <typename T, size_t N>
  struct remove_const<const T[N]> {
    using type = T[N];
  };

  template <typename T>
  using remove_const_t = typename remove_const<T>::type;

  /////////////////////
  // remove_volatile //
  /////////////////////

  template <typename T>
  struct remove_volatile {
    using type = T;
  };
  template <typename T>
  struct remove_volatile<volatile T> {
    using type = T;
  };
  template <typename T>
  struct remove_volatile<volatile T[]> {
    using type = T[];
  };
  template <typename T, size_t N>
  struct remove_volatile<volatile T[N]> {
    using type = T[N];
  };

  template <typename T>
  using remove_volatile_t = typename remove_volatile<T>::type;

  ///////////////
  // remove_cv //
  ///////////////

  template <typename T>
  struct remove_cv {
    using type = remove_volatile_t<remove_const_t<T>>;
  };

  template <typename T>
  using remove_cv_t = typename remove_cv<T>::type;

  //////////////////////
  // remove_reference //
  //////////////////////

  template <typename T>
  struct remove_reference {
    using type = T;
  };
  template <typename T>
  struct remove_reference<T &> {
    using type = T;
  };
  template <typename T>
  struct remove_reference<T &&> {
    using type = T;
  };

  template <typename T>
  using remove_reference_t = typename remove_reference<T>::type;

  //////////////////
  // remove_cvref //
  //////////////////

  template <typename T>
  struct remove_cvref {
    using type = remove_volatile_t<remove_const_t<remove_reference_t<T>>>;
  };

  template <typename T>
  using remove_cvref_t = typename remove_cvref<T>::type;

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
      : decltype(internal::try_add_lvalue_reference<T>(0)) {};

  template <typename T>
  using add_lvalue_reference_t = typename add_lvalue_reference<T>::type;

  //////////////////////////
  // add_rvalue_reference //
  //////////////////////////

  namespace internal
  {
    template <typename T>
    auto try_add_rvalue_reference(int) -> type_identity<T &&>;

    template <typename T>
    auto try_add_rvalue_reference(...) -> type_identity<T>;
  } // namespace internal

  template <typename T>
  struct add_rvalue_reference
      : decltype(internal::try_add_rvalue_reference<T>(0)) {};

  template <typename T>
  using add_rvalue_reference_t = typename add_rvalue_reference<T>::type;

  /////////////
  // declval //
  /////////////

  template <typename T>
  typename add_rvalue_reference<T>::type declval() noexcept;

  ////////////
  // endian //
  ////////////

  enum class endian {
    little = __ORDER_LITTLE_ENDIAN__,
    big = __ORDER_BIG_ENDIAN__,
    native = __BYTE_ORDER__,
  };
} // namespace NS_NOSTL

// clang-format off
#include "internal/type_fundamental.h"
#include "internal/type_transformations.h"
#include "internal/type_void_t.h"
#include "internal/type_properties.h"
#include "internal/type_compound.h"
#include "internal/type_pod.h"
// clang-format on
