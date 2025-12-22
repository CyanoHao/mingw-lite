#pragma once

#include "_dll.h"

#include <stringapiset.h>

namespace mingw_thunk
{
#define __DECLARE_NON_THUNK_KERNEL32_FUNCTION(name)                            \
  inline auto *try_get_##name() noexcept                                       \
  {                                                                            \
    static auto *pfn =                                                         \
        internal::module_kernel32().get_function<decltype(::name)>(#name);     \
    return pfn;                                                                \
  }

  namespace
  {
    __DECLARE_NON_THUNK_KERNEL32_FUNCTION(MultiByteToWideChar)
    __DECLARE_NON_THUNK_KERNEL32_FUNCTION(WideCharToMultiByte)
  } // namespace

#undef __DECLARE_NON_THUNK_KERNEL32_FUNCTION

} // namespace mingw_thunk
