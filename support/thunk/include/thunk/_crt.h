#pragma once

#include "_common.h"
#include "_dll.h"

#include <io.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/utime.h>

namespace mingw_thunk
{
  namespace internal
  {
    inline const module_handle &module_crt() noexcept
    {
#if defined(_UCRT)
      static module_handle module{"ucrtbase.dll"};
#else
      static module_handle module{"msvcrt.dll"};
#endif
      return module;
    }

  } // namespace internal

#define __DECLARE_CRT_IMP_ALIAS(symbol, function)                              \
  auto *dllimport_##symbol __asm__(                                            \
      __DLLIMPORT_SYMBOL_NAME___cdecl(symbol, 0)) = &function;

#define __DECLARE_NON_THUNK_CRT_FUNCTION(name)                                 \
  inline auto crt_##name() noexcept                                            \
  {                                                                            \
    static auto *pfn =                                                         \
        internal::module_crt().get_function<decltype(::name)>(#name);          \
    return pfn;                                                                \
  }

  __DECLARE_NON_THUNK_CRT_FUNCTION(fopen);

#undef __DECLARE_NON_THUNK_CRT_FUNCTION

#define __DECLARE_REDIRECTED_CRT_FUNCTION(function, export)                    \
  inline auto crt_##function() noexcept                                        \
  {                                                                            \
    static auto *pfn =                                                         \
        internal::module_crt().get_function<decltype(::function)>(export);     \
    return pfn;                                                                \
  }

#if defined(_UCRT)

  __DECLARE_REDIRECTED_CRT_FUNCTION(_findfirst32, "_findfirst32")
  __DECLARE_REDIRECTED_CRT_FUNCTION(_findfirst32i64, "_findfirst32i64")
  __DECLARE_REDIRECTED_CRT_FUNCTION(_findfirst64, "_findfirst64")
  __DECLARE_REDIRECTED_CRT_FUNCTION(_findfirst64i32, "_findfirst64i32")
  __DECLARE_REDIRECTED_CRT_FUNCTION(_findnext32, "_findnext32")
  __DECLARE_REDIRECTED_CRT_FUNCTION(_findnext32i64, "_findnext32i64")
  __DECLARE_REDIRECTED_CRT_FUNCTION(_findnext64, "_findnext64")
  __DECLARE_REDIRECTED_CRT_FUNCTION(_findnext64i32, "_findnext64i32")
  __DECLARE_REDIRECTED_CRT_FUNCTION(_fstat32, "_fstat32")
  __DECLARE_REDIRECTED_CRT_FUNCTION(_fstat32i64, "_fstat32i64")
  __DECLARE_REDIRECTED_CRT_FUNCTION(_fstat64, "_fstat64")
  __DECLARE_REDIRECTED_CRT_FUNCTION(_fstat64i32, "_fstat64i32")
  __DECLARE_REDIRECTED_CRT_FUNCTION(_stat32, "_stat32")
  __DECLARE_REDIRECTED_CRT_FUNCTION(_stat32i64, "_stat32i64")
  __DECLARE_REDIRECTED_CRT_FUNCTION(_stat64, "_stat64")
  __DECLARE_REDIRECTED_CRT_FUNCTION(_stat64i32, "_stat64i32")
  __DECLARE_REDIRECTED_CRT_FUNCTION(_wfindfirst32, "_wfindfirst32")
  __DECLARE_REDIRECTED_CRT_FUNCTION(_wfindfirst32i64, "_wfindfirst32i64")
  __DECLARE_REDIRECTED_CRT_FUNCTION(_wfindfirst64, "_wfindfirst64")
  __DECLARE_REDIRECTED_CRT_FUNCTION(_wfindfirst64i32, "_wfindfirst64i32")
  __DECLARE_REDIRECTED_CRT_FUNCTION(_wfindnext32, "_wfindnext32")
  __DECLARE_REDIRECTED_CRT_FUNCTION(_wfindnext32i64, "_wfindnext32i64")
  __DECLARE_REDIRECTED_CRT_FUNCTION(_wfindnext64, "_wfindnext64")
  __DECLARE_REDIRECTED_CRT_FUNCTION(_wfindnext64i32, "_wfindnext64i32")
  __DECLARE_REDIRECTED_CRT_FUNCTION(_wstat32, "_wstat32")
  __DECLARE_REDIRECTED_CRT_FUNCTION(_wstat32i64, "_wstat32i64")
  __DECLARE_REDIRECTED_CRT_FUNCTION(_wstat64, "_wstat64")
  __DECLARE_REDIRECTED_CRT_FUNCTION(_wstat64i32, "_wstat64i32")

#elif defined(_WIN64)

#else

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

#endif

#undef __DECLARE_REDIRECTED_CRT_FUNCTION

} // namespace mingw_thunk

#define __DEFINE_CRT_THUNK(return_type, function, ...)                         \
  __DEFINE_THUNK(crt, 0, return_type, __cdecl, function, __VA_ARGS__)
