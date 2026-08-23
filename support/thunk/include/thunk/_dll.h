#pragma once

#include <libloaderapi.h>

extern "C"
{
  __attribute__((dllimport)) decltype(::LoadLibraryA) __ms_LoadLibraryA;
  __attribute__((dllimport)) decltype(::LoadLibraryW) __ms_LoadLibraryW;
}

namespace mingw_thunk::internal
{
  struct module_handle
  {
    HMODULE module;

#if THUNK_LEVEL >= NTDDI_WIN4
    module_handle(const wchar_t *module_name) noexcept
    {
      module = __ms_LoadLibraryW(module_name);
    }
#else
    module_handle(const char *module_name) noexcept
    {
      module = __ms_LoadLibraryA(module_name);
    }
#endif

    module_handle(const module_handle &) = delete;

    ~module_handle() noexcept
    {
      if (module)
        FreeLibrary(module);
    }

    template <typename Fn>
    Fn *get_function(const char *function_name) const noexcept
    {
      return module
                 ? reinterpret_cast<Fn *>(GetProcAddress(module, function_name))
                 : nullptr;
    }
  };

#if THUNK_LEVEL >= NTDDI_WIN4
#define __DECLARE_SIMPLE_EXPLICIT_DLL_MODULE(name)                             \
  inline const module_handle &module_##name()                                  \
  {                                                                            \
    static module_handle module{L"" #name ".dll"};                             \
    return module;                                                             \
  }
#else
#define __DECLARE_SIMPLE_EXPLICIT_DLL_MODULE(name)                             \
  inline const module_handle &module_##name()                                  \
  {                                                                            \
    static module_handle module{#name ".dll"};                                 \
    return module;                                                             \
  }
#endif

  __DECLARE_SIMPLE_EXPLICIT_DLL_MODULE(advapi32)
  __DECLARE_SIMPLE_EXPLICIT_DLL_MODULE(bcrypt)
  __DECLARE_SIMPLE_EXPLICIT_DLL_MODULE(crypt32)
  __DECLARE_SIMPLE_EXPLICIT_DLL_MODULE(kernel32)
  __DECLARE_SIMPLE_EXPLICIT_DLL_MODULE(msvcrt)
  __DECLARE_SIMPLE_EXPLICIT_DLL_MODULE(ntdll)
  __DECLARE_SIMPLE_EXPLICIT_DLL_MODULE(psapi)
  __DECLARE_SIMPLE_EXPLICIT_DLL_MODULE(shell32)
  __DECLARE_SIMPLE_EXPLICIT_DLL_MODULE(user32)
  __DECLARE_SIMPLE_EXPLICIT_DLL_MODULE(ws2_32)

#undef __DECLARE_SIMPLE_EXPLICIT_DLL_MODULE

#if THUNK_LEVEL >= NTDDI_WIN4
#define __DECLARE_CANONICAL_EXPLICIT_DLL_MODULE(identifier, filename)          \
  inline const module_handle &module_##identifier()                            \
  {                                                                            \
    static module_handle module{L"" filename};                                 \
    return module;                                                             \
  }
#else
#define __DECLARE_CANONICAL_EXPLICIT_DLL_MODULE(identifier, filename)          \
  inline const module_handle &module_##identifier()                            \
  {                                                                            \
    static module_handle module{filename};                                     \
    return module;                                                             \
  }
#endif

  __DECLARE_CANONICAL_EXPLICIT_DLL_MODULE(api_ms_win_core_path_l1_1_0,
                                          "api-ms-win-core-path-l1-1-0.dll")
  __DECLARE_CANONICAL_EXPLICIT_DLL_MODULE(api_ms_win_crt_convert_l1_1_0,
                                          "api-ms-win-crt-convert-l1-1-0.dll")
  __DECLARE_CANONICAL_EXPLICIT_DLL_MODULE(
      api_ms_win_crt_environment_l1_1_0,
      "api-ms-win-crt-environment-l1-1-0.dll")
  __DECLARE_CANONICAL_EXPLICIT_DLL_MODULE(
      api_ms_win_crt_filesystem_l1_1_0, "api-ms-win-crt-filesystem-l1-1-0.dll")
  __DECLARE_CANONICAL_EXPLICIT_DLL_MODULE(api_ms_win_crt_locale_l1_1_0,
                                          "api-ms-win-crt-locale-l1-1-0.dll")
  __DECLARE_CANONICAL_EXPLICIT_DLL_MODULE(api_ms_win_crt_runtime_l1_1_0,
                                          "api-ms-win-crt-runtime-l1-1-0.dll")
  __DECLARE_CANONICAL_EXPLICIT_DLL_MODULE(api_ms_win_crt_stdio_l1_1_0,
                                          "api-ms-win-crt-stdio-l1-1-0.dll")
  __DECLARE_CANONICAL_EXPLICIT_DLL_MODULE(api_ms_win_crt_string_l1_1_0,
                                          "api-ms-win-crt-string-l1-1-0.dll")
  __DECLARE_CANONICAL_EXPLICIT_DLL_MODULE(api_ms_win_crt_time_l1_1_0,
                                          "api-ms-win-crt-time-l1-1-0.dll")

#undef __DECLARE_CANONICAL_EXPLICIT_DLL_MODULE

} // namespace mingw_thunk::internal
