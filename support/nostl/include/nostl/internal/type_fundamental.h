#pragma once

#include "config.h"

namespace NS_NOSTL
{
  /////////////
  // is_void //
  /////////////

  template <typename T>
  struct is_void : false_type {};
  template <>
  struct is_void<void> : true_type {};
  template <>
  struct is_void<void const> : true_type {};
  template <>
  struct is_void<void volatile> : true_type {};
  template <>
  struct is_void<void const volatile> : true_type {};

  template <typename T>
  constexpr bool is_void_v = is_void<T>::value;

  //////////////////
  // has_void_arg //
  //////////////////

  template <typename... Args>
  struct has_void_arg : bool_constant<(... || is_void_v<Args>)> {};

  template <typename... Args>
  constexpr bool has_void_arg_v = has_void_arg<Args...>::value;

  /////////////////////
  // is_null_pointer //
  /////////////////////

  template <typename T>
  struct is_null_pointer : is_same<remove_cv_t<T>, decltype(nullptr)> {};

  template <typename T>
  constexpr bool is_null_pointer_v = is_null_pointer<T>::value;

  /////////////////
  // is_integral //
  /////////////////

  namespace internal
  {
    template <typename T>
    struct is_integral_helper : false_type {};

    template <>
    struct is_integral_helper<unsigned char> : true_type {};
    template <>
    struct is_integral_helper<unsigned short> : true_type {};
    template <>
    struct is_integral_helper<unsigned int> : true_type {};
    template <>
    struct is_integral_helper<unsigned long> : true_type {};
    template <>
    struct is_integral_helper<unsigned long long> : true_type {};

    template <>
    struct is_integral_helper<signed char> : true_type {};
    template <>
    struct is_integral_helper<signed short> : true_type {};
    template <>
    struct is_integral_helper<signed int> : true_type {};
    template <>
    struct is_integral_helper<signed long> : true_type {};
    template <>
    struct is_integral_helper<signed long long> : true_type {};

    template <>
    struct is_integral_helper<bool> : true_type {};
    template <>
    struct is_integral_helper<char> : true_type {};

#ifdef __cpp_char8_t
    template <>
    struct is_integral_helper<char8_t> : true_type {};
#endif
    template <>
    struct is_integral_helper<char16_t> : true_type {};
    template <>
    struct is_integral_helper<char32_t> : true_type {};
    template <>
    struct is_integral_helper<wchar_t> : true_type {};

#ifdef __SIZEOF_INT128__
    template <>
    struct is_integral_helper<__int128> : true_type {};
    template <>
    struct is_integral_helper<unsigned __int128> : true_type {};
#endif
  } // namespace internal

  template <typename T>
  struct is_integral : internal::is_integral_helper<remove_cv_t<T>> {};

  template <typename T>
  constexpr bool is_integral_v = is_integral<T>::value;

  ///////////////////////
  // is_floating_point //
  ///////////////////////

  namespace internal
  {
    template <typename T>
    struct is_floating_point_helper : false_type {};

    template <>
    struct is_floating_point_helper<float> : true_type {};
    template <>
    struct is_floating_point_helper<double> : true_type {};
    template <>
    struct is_floating_point_helper<long double> : true_type {};

#ifdef __SIZEOF_FLOAT128__
    template <>
    struct is_floating_point_helper<__float128> : true_type {};
#endif
  } // namespace internal

  template <typename T>
  struct is_floating_point
      : internal::is_floating_point_helper<remove_cv_t<T>> {};

  template <typename T>
  constexpr bool is_floating_point_v = is_floating_point<T>::value;

  ///////////////////
  // is_arithmetic //
  ///////////////////

  template <typename T>
  struct is_arithmetic
      : bool_constant<is_integral_v<T> || is_floating_point_v<T>> {};

  template <typename T>
  constexpr bool is_arithmetic_v = is_arithmetic<T>::value;

  ////////////////////
  // is_fundamental //
  ////////////////////

  template <typename T>
  struct is_fundamental
      : public bool_constant<is_void_v<T> || is_integral_v<T> ||
                             is_floating_point_v<T> || is_null_pointer_v<T>> {};

  template <typename T>
  constexpr bool is_fundamental_v = is_fundamental<T>::value;

  /////////////
  // is_enum //
  /////////////

  template <typename T>
  struct is_enum : bool_constant<__is_enum(T)> {};

  template <typename T>
  constexpr bool is_enum_v = is_enum<T>::value;

} // namespace NS_NOSTL
