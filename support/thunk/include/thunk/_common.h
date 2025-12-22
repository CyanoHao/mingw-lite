#pragma once

#include "_dll.h"
#include "test.h"

#ifdef ENABLE_TEST_OVERRIDE

#define __DECLARE_TRY_GET_FUNCTION(function)                                   \
  namespace                                                                    \
  {                                                                            \
    inline fn_##function##_t *try_get_##function() noexcept                    \
    {                                                                          \
      internal::touched = true;                                                \
      return nullptr;                                                          \
    }                                                                          \
  }

#else // ifdef ENABLE_TEST_OVERRIDE

#define __DECLARE_TRY_GET_FUNCTION(function)                                   \
  inline auto *try_get_##function() noexcept                                   \
  {                                                                            \
    return get_##function();                                                   \
  }

#endif // ifdef ENABLE_TEST_OVERRIDE

#ifdef _WIN64

#define __DLLIMPORT_SYMBOL_NAME___cdecl(function, size) "__imp_" #function
#define __DLLIMPORT_SYMBOL_NAME___stdcall(function, size) "__imp_" #function

#else // ifdef _WIN64

#define __DLLIMPORT_SYMBOL_NAME___cdecl(function, size) "__imp__" #function
#define __DLLIMPORT_SYMBOL_NAME___stdcall(function, size)                      \
  "__imp__" #function "@" #size

#endif // ifdef _WIN64

#define __DLLIMPORT_SYMBOL_NAME_APIENTRY(function, size)                       \
  __DLLIMPORT_SYMBOL_NAME___stdcall(function, size)
#define __DLLIMPORT_SYMBOL_NAME_WINAPI(function, size)                         \
  __DLLIMPORT_SYMBOL_NAME___stdcall(function, size)
#define __DLLIMPORT_SYMBOL_NAME_WSAAPI(function, size)                         \
  __DLLIMPORT_SYMBOL_NAME___stdcall(function, size)

#define __DEFINE_THUNK(                                                        \
    module, size, return_type, calling_convention, function, ...)              \
                                                                               \
  extern "C" return_type calling_convention function(__VA_ARGS__);             \
                                                                               \
  using fn_##function##_t = decltype(function);                                \
                                                                               \
  namespace                                                                    \
  {                                                                            \
    inline auto *get_##function() noexcept                                     \
    {                                                                          \
      static auto *pfn =                                                       \
          internal::module_##module().get_function<fn_##function##_t>(           \
              #function);                                                      \
      return pfn;                                                              \
    }                                                                          \
                                                                               \
    __DECLARE_TRY_GET_FUNCTION(function)                                       \
  }                                                                            \
                                                                               \
  fn_##function##_t *dllimport_##function __asm__(                             \
      __DLLIMPORT_SYMBOL_NAME_##calling_convention(function, size)) =          \
      function;                                                                \
                                                                               \
  extern "C" return_type calling_convention function(__VA_ARGS__)
