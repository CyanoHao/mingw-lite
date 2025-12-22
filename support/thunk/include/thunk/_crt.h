#pragma once

#include "_common.h"
#include "_dll.h"

#include <io.h>

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

#define __DECLARE_REDIRECTED_CRT_FUNCTION(function, export)                    \
  inline auto crt_get_##function() noexcept                                    \
  {                                                                            \
    static auto *pfn =                                                         \
        internal::module_crt().get_function<decltype(::function)>(export);     \
    return pfn;                                                                \
  }

#if defined(_UCRT)

  __DECLARE_REDIRECTED_CRT_FUNCTION(_wfindfirst32, "_wfindfirst32");
  __DECLARE_REDIRECTED_CRT_FUNCTION(_wfindfirst32i64, "_wfindfirst32i64");
  __DECLARE_REDIRECTED_CRT_FUNCTION(_wfindfirst64, "_wfindfirst64");
  __DECLARE_REDIRECTED_CRT_FUNCTION(_wfindfirst64i32, "_wfindfirst64i32");
  __DECLARE_REDIRECTED_CRT_FUNCTION(_wfindnext32, "_wfindnext32");
  __DECLARE_REDIRECTED_CRT_FUNCTION(_wfindnext32i64, "_wfindnext32i64");
  __DECLARE_REDIRECTED_CRT_FUNCTION(_wfindnext64, "_wfindnext64");
  __DECLARE_REDIRECTED_CRT_FUNCTION(_wfindnext64i32, "_wfindnext64i32");

#elif defined(_WIN64)

  __DECLARE_REDIRECTED_CRT_FUNCTION(_wfindfirst64, "_wfindfirst64");
  __DECLARE_REDIRECTED_CRT_FUNCTION(_wfindfirst64i32, "_wfindfirst");
  __DECLARE_REDIRECTED_CRT_FUNCTION(_wfindnext64, "_wfindnext64");
  __DECLARE_REDIRECTED_CRT_FUNCTION(_wfindnext64i32, "_wfindnext");
  // symbol _wfindfirsti64 is identical to _wfindfirst64

#else

  __DECLARE_REDIRECTED_CRT_FUNCTION(_wfindfirst32, "_wfindfirst");
  __DECLARE_REDIRECTED_CRT_FUNCTION(_wfindfirst32i64, "_wfindfirsti64");
  // symbol _wfindfirst64 is not available in all versions

#endif

#undef __DECLARE_REDIRECTED_CRT_FUNCTION

} // namespace mingw_thunk

#define __DEFINE_CRT_THUNK(return_type, function, ...)                         \
  __DEFINE_THUNK(crt, 0, return_type, __cdecl, function, __VA_ARGS__)
