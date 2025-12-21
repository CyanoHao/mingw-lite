#pragma once

#include "_dll.h"

#include <windows.h>

#include <io.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/utime.h>

namespace mingw_thunk
{
#define __DECLARE_NON_THUNK_FUNCTION(module, name)                             \
  inline auto *module##_##name() noexcept                                      \
  {                                                                            \
    static auto *pfn =                                                         \
        internal::module_##module().get_function<decltype(::name)>(#name);     \
    return pfn;                                                                \
  }

  namespace
  {
    __DECLARE_NON_THUNK_FUNCTION(kernel32, CopyFileA)
    __DECLARE_NON_THUNK_FUNCTION(kernel32, CreateDirectoryA)
    __DECLARE_NON_THUNK_FUNCTION(kernel32, CreateEventA)
    __DECLARE_NON_THUNK_FUNCTION(kernel32, CreateFileA)
    __DECLARE_NON_THUNK_FUNCTION(kernel32, CreateProcessA)
    __DECLARE_NON_THUNK_FUNCTION(kernel32, CreateSemaphoreA)
    __DECLARE_NON_THUNK_FUNCTION(kernel32, CreateWaitableTimerA)
    __DECLARE_NON_THUNK_FUNCTION(kernel32, DeleteFileA)
    __DECLARE_NON_THUNK_FUNCTION(kernel32, FindFirstFileA)
    __DECLARE_NON_THUNK_FUNCTION(kernel32, FindNextFileA)
    __DECLARE_NON_THUNK_FUNCTION(kernel32, GetCurrentDirectoryA)
    __DECLARE_NON_THUNK_FUNCTION(kernel32, GetEnvironmentVariableA)
    __DECLARE_NON_THUNK_FUNCTION(kernel32, GetFileAttributesA)
    __DECLARE_NON_THUNK_FUNCTION(kernel32, GetFileAttributesExA)
    __DECLARE_NON_THUNK_FUNCTION(kernel32, GetFullPathNameA)
    __DECLARE_NON_THUNK_FUNCTION(kernel32, GetModuleFileNameA)
    __DECLARE_NON_THUNK_FUNCTION(kernel32, GetTempPathA)
    __DECLARE_NON_THUNK_FUNCTION(kernel32, RemoveDirectoryA)
    __DECLARE_NON_THUNK_FUNCTION(kernel32, SetCurrentDirectoryA)
    __DECLARE_NON_THUNK_FUNCTION(kernel32, SetEnvironmentVariableA)
    __DECLARE_NON_THUNK_FUNCTION(kernel32, WriteConsoleA)

    __DECLARE_NON_THUNK_FUNCTION(msvcrt, fopen);
  } // namespace

#undef __DECLARE_NON_THUNK_FUNCTION

#define __DECLARE_LOAD_TIME_IMPORT(name)                                       \
  extern "C" __attribute__((dllimport)) decltype(::name) __ms_##name;

  __DECLARE_LOAD_TIME_IMPORT(CreateProcessA)
  __DECLARE_LOAD_TIME_IMPORT(CreateWaitableTimerA)
  __DECLARE_LOAD_TIME_IMPORT(MultiByteToWideChar)
  __DECLARE_LOAD_TIME_IMPORT(WideCharToMultiByte)

#undef __DECLARE_LOAD_TIME_IMPORT

#ifndef _WIN64

#define __DECLARE_REDIRECTED_CRT_FUNCTION(function, export)                    \
  inline auto *msvcrt_##function() noexcept                                    \
  {                                                                            \
    static auto *pfn =                                                         \
        internal::module_msvcrt().get_function<decltype(::function)>(export);  \
    return pfn;                                                                \
  }

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

#undef __DECLARE_REDIRECTED_CRT_FUNCTION

#endif

} // namespace mingw_thunk
