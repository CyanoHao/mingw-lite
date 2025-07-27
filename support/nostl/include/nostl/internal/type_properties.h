#pragma once

#include "config.h"

namespace NS_NOSTL
{
  /////////////////////
  // underlying_type //
  /////////////////////

  template <typename T>
  struct underlying_type : type_identity<__underlying_type(T)> {};

  template <typename T>
  using underlying_type_t = typename underlying_type<T>::type;

  ///////////////////
  // to_underlying //
  ///////////////////

  template <typename T>
  constexpr underlying_type_t<T> to_underlying(T value) noexcept
  {
    return static_cast<underlying_type_t<T>>(value);
  }

  ///////////////////////////////////////
  // has_unique_object_representations //
  ///////////////////////////////////////

  template <typename T>
  struct has_unique_object_representations
      : bool_constant<__has_unique_object_representations(
            remove_cv_t<remove_all_extents_t<T>>)> {};

  template <typename T>
  constexpr bool has_unique_object_representations_v =
      has_unique_object_representations<T>::value;

  ///////////////
  // is_signed //
  ///////////////

  namespace internal
  {
    template <typename T, bool = is_arithmetic_v<T>>
    struct is_signed_helper : bool_constant<T(-1) < T(0)> {};
    template <typename T>
    struct is_signed_helper<T, false> : false_type {};
  } // namespace internal

  template <typename T>
  struct is_signed : internal::is_signed_helper<T> {};

  template <typename T>
  constexpr bool is_signed_v = is_signed<T>::value;

  /////////////////
  // is_unsigned //
  /////////////////

  template <typename T>
  struct is_unsigned : bool_constant<is_arithmetic_v<T> && !is_signed_v<T>> {};

  template <typename T>
  constexpr bool is_unsigned_v = is_unsigned<T>::value;

  //////////////////////
  // is_bounded_array //
  //////////////////////

  template <typename T>
  struct is_bounded_array : false_type {};
  template <typename T, size_t N>
  struct is_bounded_array<T[N]> : true_type {};

  template <typename T>
  constexpr bool is_bounded_array_v = is_bounded_array<T>::value;

  ////////////////////////
  // is_unbounded_array //
  ////////////////////////

  template <typename T>
  struct is_unbounded_array : false_type {};
  template <typename T>
  struct is_unbounded_array<T[]> : true_type {};

  template <typename T>
  constexpr bool is_unbounded_array_v = is_unbounded_array<T>::value;

  //////////////////
  // alignment_of //
  //////////////////

  template <typename T>
  struct alignment_of : integral_constant<size_t, alignof(T)> {};

  template <typename T>
  constexpr size_t alignment_of_v = alignment_of<T>::value;

  //////////
  // rank //
  //////////

  template <typename T>
  struct rank : integral_constant<size_t, 0> {};
  template <typename T>
  struct rank<T[]> : integral_constant<size_t, rank<T>::value + 1> {};
  template <typename T, size_t N>
  struct rank<T[N]> : integral_constant<size_t, rank<T>::value + 1> {};

  template <typename T>
  constexpr size_t rank_v = rank<T>::value;

  ////////////////
  // is_base_of //
  ////////////////

  template <typename Base, typename Derived>
  struct is_base_of : bool_constant<__is_base_of(Base, Derived)> {};

  template <typename Base, typename Derived>
  constexpr bool is_base_of_v = is_base_of<Base, Derived>::value;

  /////////////////////////
  // is_lvalue_reference //
  /////////////////////////

  template <typename T>
  struct is_lvalue_reference : false_type {};
  template <typename T>
  struct is_lvalue_reference<T &> : true_type {};

  template <typename T>
  constexpr bool is_lvalue_reference_v = is_lvalue_reference<T>::value;

  /////////////////////////
  // is_rvalue_reference //
  /////////////////////////

  template <typename T>
  struct is_rvalue_reference : false_type {};
  template <typename T>
  struct is_rvalue_reference<T &&> : true_type {};

  template <typename T>
  constexpr bool is_rvalue_reference_v = is_rvalue_reference<T>::value;

  //////////////////
  // has_equality //
  //////////////////

  template <typename, typename = void_t<>>
  struct has_equality : false_type {};
  template <typename T>
  struct has_equality<T, void_t<decltype(declval<T>() == declval<T>())>>
      : true_type {};

  template <typename T>
  constexpr bool has_equality_v = has_equality<T>::value;

} // namespace NS_NOSTL
