#pragma once

#include "config.h"

namespace NS_NOSTL
{
  ///////////////
  // add_const //
  ///////////////

  template <typename T>
  struct add_const {
    using type = const T;
  };

  template <typename T>
  using add_const_t = typename add_const<T>::type;

  //////////////////
  // add_volatile //
  //////////////////

  template <typename T>
  struct add_volatile {
    using type = volatile T;
  };

  template <typename T>
  using add_volatile_t = typename add_volatile<T>::type;

  ////////////
  // add_cv //
  ////////////

  template <typename T>
  struct add_cv {
    using type = const volatile T;
  };

  template <typename T>
  using add_cv_t = typename add_cv<T>::type;

  ////////////////////
  // remove_pointer //
  ////////////////////

  template <typename T>
  struct remove_pointer : type_identity<T> {};
  template <typename T>
  struct remove_pointer<T *> : type_identity<T> {};
  template <typename T>
  struct remove_pointer<T *const> : type_identity<T> {};
  template <typename T>
  struct remove_pointer<T *volatile> : type_identity<T> {};
  template <typename T>
  struct remove_pointer<T *const volatile> : type_identity<T> {};

  template <typename T>
  using remove_pointer_t = typename remove_pointer<T>::type;

  /////////////////
  // add_pointer //
  /////////////////

  namespace internal
  {
    template <typename T>
    auto try_add_pointer(int) -> type_identity<remove_reference_t<T> *>;
    template <typename T>
    auto try_add_pointer(...) -> type_identity<T>;
  } // namespace internal

  template <typename T>
  struct add_pointer : decltype(internal::try_add_pointer<T>(0)) {};

  template <typename T>
  using add_pointer_t = typename add_pointer<T>::type;

  ///////////////////
  // remove_extent //
  ///////////////////

  template <typename T>
  struct remove_extent : type_identity<T> {};
  template <typename T>
  struct remove_extent<T[]> : type_identity<T> {};
  template <typename T, size_t N>
  struct remove_extent<T[N]> : type_identity<T> {};

  template <typename T>
  using remove_extent_t = typename remove_extent<T>::type;

  ////////////////////////
  // remove_all_extents //
  ////////////////////////

  template <typename T>
  struct remove_all_extents : type_identity<T> {};
  template <typename T>
  struct remove_all_extents<T[]> : remove_all_extents<T> {};
  template <typename T, size_t N>
  struct remove_all_extents<T[N]> : remove_all_extents<T> {};

  template <typename T>
  using remove_all_extents_t = typename remove_all_extents<T>::type;

} // namespace NS_NOSTL
