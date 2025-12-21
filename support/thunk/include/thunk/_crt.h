#pragma once

#include "_common.h"
#include "_dll.h"

#include <io.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/utime.h>

namespace mingw_thunk
{
#if defined(_UCRT)

#define __DECLARE_CANONICAL_EXPLICIT_DLL_MODULE(identifier, filename)          \
  inline const module_handle &module_##identifier()                            \
  {                                                                            \
    static module_handle module{filename};                                     \
    return module;                                                             \
  }

  namespace internal
  {
    __DECLARE_CANONICAL_EXPLICIT_DLL_MODULE(
        api_ms_win_crt_filesystem_l1_1_0,
        "api-ms-win-crt-filesystem-l1-1-0.dll")
    __DECLARE_CANONICAL_EXPLICIT_DLL_MODULE(api_ms_win_crt_stdio_l1_1_0,
                                            "api-ms-win-crt-stdio-l1-1-0.dll")
  } // namespace internal

#undef __DECLARE_CANONICAL_EXPLICIT_DLL_MODULE

#else

  namespace internal
  {
    inline const module_handle &module_vcrt()
    {
      static module_handle module{"msvcrt.dll"};
      return module;
    }
  } // namespace internal

#endif

#if !defined(_UCRT)

#define __DECLARE_NON_THUNK_CRT_FUNCTION(name)                                 \
  inline auto vcrt_##name() noexcept                                           \
  {                                                                            \
    static auto *pfn =                                                         \
        internal::module_vcrt().get_function<decltype(::name)>(#name);         \
    return pfn;                                                                \
  }

  __DECLARE_NON_THUNK_CRT_FUNCTION(fopen);

#undef __DECLARE_NON_THUNK_CRT_FUNCTION

#define __DECLARE_REDIRECTED_CRT_FUNCTION(function, export)                    \
  inline auto vcrt_##function() noexcept                                       \
  {                                                                            \
    static auto *pfn =                                                         \
        internal::module_vcrt().get_function<decltype(::function)>(export);    \
    return pfn;                                                                \
  }

#if defined(_WIN64)

  namespace
  {
  } // namespace

#else

  namespace
  {
    __DECLARE_REDIRECTED_CRT_FUNCTION(_findfirst32, "_findfirst")
    __DECLARE_REDIRECTED_CRT_FUNCTION(_findfirst32i64, "_findfirsti64")
    __DECLARE_REDIRECTED_CRT_FUNCTION(_findnext32, "_findnext")
    __DECLARE_REDIRECTED_CRT_FUNCTION(_findnext32i64, "_findnexti64")
    __DECLARE_REDIRECTED_CRT_FUNCTION(_stat32, "_stat")
    __DECLARE_REDIRECTED_CRT_FUNCTION(_stat32i64, "_stati64")
    __DECLARE_REDIRECTED_CRT_FUNCTION(_utime32, "_utime")
    __DECLARE_REDIRECTED_CRT_FUNCTION(_wfindfirst32, "_wfindfirst")
    __DECLARE_REDIRECTED_CRT_FUNCTION(_wfindfirst32i64, "_wfindfirsti64")
    __DECLARE_REDIRECTED_CRT_FUNCTION(_wfindnext32, "_wfindnext")
    __DECLARE_REDIRECTED_CRT_FUNCTION(_wfindnext32i64, "_wfindnexti64")
    __DECLARE_REDIRECTED_CRT_FUNCTION(_wstat32, "_wstat")
    __DECLARE_REDIRECTED_CRT_FUNCTION(_wstat32i64, "_wstati64")
    __DECLARE_REDIRECTED_CRT_FUNCTION(_wutime32, "_wutime")
  } // namespace

#endif

#undef __DECLARE_REDIRECTED_CRT_FUNCTION

#endif

} // namespace mingw_thunk

#if defined(_UCRT)

#define __DEFINE_UCRT_THUNK(module, return_type, function, ...)                \
  __DEFINE_THUNK(module, 0, return_type, __cdecl, function, __VA_ARGS__)

#else

#define __DEFINE_VCRT_THUNK(return_type, function, ...)                        \
  __DEFINE_THUNK(vcrt, 0, return_type, __cdecl, function, __VA_ARGS__)

#endif

#define __DECLARE_CRT_IMP_ALIAS(symbol, function)                              \
  auto *dllimport_##symbol __asm__(                                            \
      __DLLIMPORT_SYMBOL_NAME___cdecl(symbol, 0)) = &function;
