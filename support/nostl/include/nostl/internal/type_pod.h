#pragma once

#include "config.h"

#include "move_help.h"

namespace NS_NOSTL
{
  //////////////
  // is_empty //
  //////////////

  template <typename T>
  struct is_empty : bool_constant<__is_empty(T)> {};

  template <typename T>
  constexpr bool is_empty_v = is_empty<T>::value;

  ////////////
  // is_pod //
  ////////////

  template <typename T>
  struct is_pod : bool_constant<__is_pod(T)> {};

  ////////////////////////
  // is_standard_layout //
  ////////////////////////

  template <typename T>
  struct is_standard_layout : bool_constant<__is_standard_layout(T)> {};

  template <typename T>
  constexpr bool is_standard_layout_v = is_standard_layout<T>::value;

  ////////////////////////////
  // has_virtual_destructor //
  ////////////////////////////

  template <typename T>
  struct has_virtual_destructor : bool_constant<__has_virtual_destructor(T)> {};

  template <typename T>
  constexpr bool has_virtual_destructor_v = has_virtual_destructor<T>::value;

  /////////////////
  // is_abstract //
  /////////////////

  template <typename T>
  struct is_abstract : bool_constant<__is_abstract(T)> {};

  template <typename T>
  constexpr bool is_abstract_v = is_abstract<T>::value;

  ///////////////////////////
  // is_trivially_copyable //
  ///////////////////////////

  template <typename T>
  struct is_trivially_copyable : bool_constant<__is_trivially_copyable(T)> {};

  template <typename T>
  constexpr bool is_trivially_copyable_v = is_trivially_copyable<T>::value;

  //////////////////////
  // is_constructible //
  //////////////////////

  namespace internal
  {
    template <typename T, typename... Args>
    auto is(T &&, Args &&...) -> decltype(move(T(declval<Args>()...)), 0);
    template <typename... Args>
    char is(argument_sink, Args &&...);

    template <typename T>
    struct can_construct_scalar_helper {
      static int can(T);
      static char can(...);
    };

    template <bool, typename T, typename... Args>
    struct is_constructible_helper_2
        : bool_constant<sizeof(is(declval<T>(), declval<Args>()...)) ==
                        sizeof(int)> {};
    template <typename T>
    struct is_constructible_helper_2<true, T> : is_scalar<T> {};
    template <typename T, typename Arg0>
    struct is_constructible_helper_2<true, T, Arg0>
        : bool_constant<sizeof(can_construct_scalar_helper<T>::can(
                            declval<Arg0>())) == sizeof(int)> {};
    template <typename T, typename Arg0, typename... Args>
    struct is_constructible_helper_2<true, T, Arg0, Args...> : false_type {};

    template <bool, typename T, typename... Args>
    struct is_constructible_helper_1
        : is_constructible_helper_2<is_scalar_v<T> || is_reference_v<T>,
                                    T,
                                    Args...> {};
    template <typename T, typename... Args>
    struct is_constructible_helper_1<true, T, Args...> : false_type {};
  } // namespace internal

  template <typename T, typename... Args>
  struct is_constructible : internal::is_constructible_helper_1<
                                (is_abstract_v<remove_all_extents_t<T>> ||
                                 is_unbounded_array_v<T> ||
                                 is_function_v<remove_all_extents_t<T>> ||
                                 has_void_arg_v<T, Args...>),
                                T,
                                Args...> {};

  namespace internal
  {
    template <typename Array, size_t N>
    struct is_constructible_helper_2<false, Array[N]>
        : is_constructible<remove_all_extents_t<Array>> {};
    template <typename Array, size_t N, typename... Args>
    struct is_constructible_helper_2<false, Array[N], Args...> : false_type {};
  } // namespace internal

  template <typename T, typename... Args>
  constexpr bool is_constructible_v = is_constructible<T, Args...>::value;

  //////////////////////////////
  // is_default_constructible //
  //////////////////////////////

  template <typename T>
  struct is_default_constructible : is_constructible<T> {};

  template <typename T>
  constexpr bool is_default_constructible_v =
      is_default_constructible<T>::value;

  ///////////////////////////
  // is_copy_constructible //
  ///////////////////////////

  template <typename T>
  struct is_copy_constructible
      : is_constructible<T, add_lvalue_reference_t<add_const_t<T>>> {};

  template <typename T>
  constexpr bool is_copy_constructible_v = is_copy_constructible<T>::value;

  ///////////////////
  // is_assignable //
  ///////////////////

  namespace internal
  {
    template <typename T, typename U>
    struct is_assignable_helper {
      template <typename T1, typename U1>
      static auto is(int) -> decltype(declval<T1>() = declval<U1>(), 0);
      template <typename, typename>
      static char is(...);

      static constexpr bool value = sizeof(is<T, U>(0)) == sizeof(int);
    };
  } // namespace internal

  template <typename T, typename U>
  struct is_assignable
      : bool_constant<internal::is_assignable_helper<T, U>::value> {};

  template <typename T, typename U>
  constexpr bool is_assignable_v = is_assignable<T, U>::value;

  /////////////////////////////
  // is_trivially_assignable //
  /////////////////////////////

  template <typename T, typename U>
  struct is_trivially_assignable
      : bool_constant<__is_trivially_assignable(T, U)> {};

  template <typename T, typename U>
  constexpr bool is_trivially_assignable_v =
      is_trivially_assignable<T, U>::value;

  ////////////////////////
  // is_copy_assignable //
  ////////////////////////

  template <typename T>
  struct is_copy_assignable
      : is_assignable<add_lvalue_reference_t<T>,
                      add_lvalue_reference_t<add_const_t<T>>> {};

  template <typename T>
  constexpr bool is_copy_assignable_v = is_copy_assignable<T>::value;

  //////////////////////////////////
  // is_trivially_copy_assignable //
  //////////////////////////////////

  template <typename T>
  struct is_trivially_copy_assignable
      : is_trivially_assignable<add_lvalue_reference_t<T>,
                                add_lvalue_reference_t<add_const_t<T>>> {};

} // namespace NS_NOSTL
