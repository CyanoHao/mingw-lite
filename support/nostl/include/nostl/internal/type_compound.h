#pragma once

#include <stddef.h>

#include "config.h"

namespace NS_NOSTL
{
  ////////////
  // extent //
  ////////////

  template <typename T, unsigned N>
  struct extent : integral_constant<size_t, 0> {};
  template <typename T>
  struct extent<T[], 0> : integral_constant<size_t, 0> {};
  template <typename T, unsigned N>
  struct extent<T[], N> : extent<T, N - 1> {};
  template <typename T, size_t I>
  struct extent<T[I], 0> : integral_constant<size_t, I> {};
  template <typename T, size_t I, unsigned N>
  struct extent<T[I], N> : extent<T, N - 1> {};

  template <typename T, unsigned N>
  constexpr size_t extent_v = extent<T, N>::value;

  //////////////
  // is_array //
  //////////////

  template <typename T>
  struct is_array : false_type {};
  template <typename T>
  struct is_array<T[]> : true_type {};
  template <typename T, unsigned N>
  struct is_array<T[N]> : true_type {};

  template <typename T>
  constexpr bool is_array_v = is_array<T>::value;

  ////////////////////////////////
  // is_member_function_pointer //
  ////////////////////////////////

  namespace internal
  {
    template <typename T>
    struct is_member_function_pointer_helper : false_type {};
    template <typename T, typename U>
    struct is_member_function_pointer_helper<T U::*> : is_function<T> {};
  } // namespace internal

  template <typename T>
  struct is_member_function_pointer
      : internal::is_member_function_pointer_helper<remove_cv_t<T>> {};

  template <typename T>
  constexpr bool is_member_function_pointer_v =
      is_member_function_pointer<T>::value;

  ///////////////////////
  // is_member_pointer //
  ///////////////////////

  namespace internal
  {
    template <typename T>
    struct is_member_pointer_helper : false_type {};
    template <typename T, typename U>
    struct is_member_pointer_helper<T U::*> : true_type {};
  } // namespace internal

  template <typename T>
  struct is_member_pointer
      : internal::is_member_pointer_helper<remove_cv_t<T>> {};

  template <typename T>
  constexpr bool is_member_pointer_v = is_member_pointer<T>::value;

  //////////////////////////////
  // is_member_object_pointer //
  //////////////////////////////

  template <typename T>
  struct is_member_object_pointer
      : bool_constant<is_member_pointer_v<T> &&
                      !is_member_function_pointer_v<T>> {};

  template <typename T>
  constexpr bool is_member_object_pointer_v =
      is_member_object_pointer<T>::value;

  ////////////////
  // is_pointer //
  ////////////////

  template <typename T>
  struct is_pointer : false_type {};
  template <typename T>
  struct is_pointer<T *> : true_type {};
  template <typename T>
  struct is_pointer<T *const> : true_type {};
  template <typename T>
  struct is_pointer<T *volatile> : true_type {};
  template <typename T>
  struct is_pointer<T *const volatile> : true_type {};

  template <typename T>
  constexpr bool is_pointer_v = is_pointer<T>::value;

  ////////////////////
  // is_convertible //
  ////////////////////

  namespace internal
  {
    template <typename From,
              typename To,
              bool = is_void_v<From> || is_function_v<To> || is_array_v<To>>
    struct is_convertible_helper : bool_constant<is_void_v<To>> {};
    template <typename From, typename To>
    struct is_convertible_helper<From, To, false> {
      template <typename To1>
      static void ToFunction(To1);

      template <typename, typename>
      static char is(...);
      template <typename From1, typename To1>
      static auto is(int) -> decltype(ToFunction<To1>(declval<From1>()), 0);

      static constexpr bool value = sizeof(is<From, To>(0)) == sizeof(int);
    };
  } // namespace internal

  template <typename From, typename To>
  struct is_convertible
      : public bool_constant<internal::is_convertible_helper<From, To>::value> {
  };

  template <typename From, typename To>
  constexpr bool is_convertible_v = is_convertible<From, To>::value;

  //////////////
  // is_union //
  //////////////

  template <typename T>
  struct is_union : bool_constant<__is_union(T)> {};

  template <typename T>
  constexpr bool is_union_v = is_union<T>::value;

  //////////////
  // is_class //
  //////////////

  template <typename T>
  struct is_class : bool_constant<__is_class(T)> {};

  template <typename T>
  constexpr bool is_class_v = is_class<T>::value;

  ////////////////////
  // is_polymorphic //
  ////////////////////

  template <typename T>
  struct is_polymorphic : bool_constant<__is_polymorphic(T)> {};

  template <typename T>
  constexpr bool is_polymorphic_v = is_polymorphic<T>::value;

  ///////////////
  // is_object //
  ///////////////

  template <typename T>
  struct is_object : bool_constant<!is_reference_v<T> && !is_void_v<T> &&
                                   !is_function_v<T>> {};

  template <typename T>
  constexpr bool is_object_v = is_object<T>::value;

  ///////////////
  // is_scalar //
  ///////////////

  template <typename T>
  struct is_scalar
      : bool_constant<is_arithmetic_v<T> || is_enum_v<T> || is_pointer_v<T> ||
                      is_member_pointer_v<T> || is_null_pointer_v<T>> {};

  template <typename T>
  constexpr bool is_scalar_v = is_scalar<T>::value;

  /////////////////
  // is_compound //
  /////////////////

  template <typename T>
  struct is_compound : bool_constant<!is_fundamental_v<T>> {};

  template <typename T>
  constexpr bool is_compound_v = is_compound<T>::value;

  ///////////
  // decay //
  ///////////

  template <typename T>
  struct decay {
    using U = remove_reference_t<T>;

    using type = conditional_t<
        is_array_v<U>,
        remove_extent_t<U> *,
        conditional_t<is_function_v<U>, add_pointer_t<U>, remove_cv_t<U>>>;
  };

  template <typename T>
  using decay_t = typename decay<T>::type;

  /////////////////
  // common_type //
  /////////////////

  template <typename...>
  struct common_type;
  template <typename T>
  struct common_type<T> {
    using type = decay_t<T>;
  };
  template <typename T, typename U>
  struct common_type<T, U> {
    using type = decay_t<decltype(true ? declval<T>() : declval<U>())>;
  };
  template <typename T, typename U, typename... V>
  struct common_type<T, U, V...> {
    using type =
        typename common_type<typename common_type<T, U>::type, V...>::type;
  };

  template <typename... T>
  using common_type_t = typename common_type<T...>::type;

  //////////////
  // is_final //
  //////////////

  template <typename T>
  struct is_final : bool_constant<__is_final(T)> {};

  template <typename T>
  constexpr bool is_final_v = is_final<T>::value;

  //////////////////
  // is_aggregate //
  //////////////////

  template <typename T>
  struct is_aggregate : bool_constant<__is_aggregate(T)> {};

  template <typename T>
  constexpr bool is_aggregate_v = is_aggregate<T>::value;

} // namespace NS_NOSTL
