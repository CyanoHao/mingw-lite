//===-- sanitizer_win_defs.h ------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Common definitions for Windows-specific code.
//
//===----------------------------------------------------------------------===//
// MinGW-lite local change: the WIN_WEAK_ALIAS / WIN_FORCE_LINK / WIN_EXPORT
// macros originally expand to MSVC ``__pragma(comment(linker, ...))`` directives
// (/alternatename, /include, /export) that only lld-link understands.  GNU ld
// silently ignores them which breaks weak-symbol semantics and archive-member
// inclusion.  When building with MinGW GCC we instead use GCC-native weak alias
// and section attributes, so none of the MSVC linker pragmas are needed.
//===----------------------------------------------------------------------===//
#ifndef SANITIZER_WIN_DEFS_H
#define SANITIZER_WIN_DEFS_H

#include "sanitizer_platform.h"
#if SANITIZER_WINDOWS

#ifndef WINAPI
#if defined(_M_IX86) || defined(__i386__)
#define WINAPI __stdcall
#else
#define WINAPI
#endif
#endif

#if defined(_M_IX86) || defined(__i386__)
#define WIN_SYM_PREFIX "_"
#else
#define WIN_SYM_PREFIX
#endif

// For MinGW, the /export: directives contain undecorated symbols, contrary to
// link/lld-link. The GNU linker doesn't support /alternatename and /include
// though, thus lld-link in MinGW mode interprets them in the same way as
// in the default mode.
#ifdef __MINGW32__
#define WIN_EXPORT_PREFIX
#else
#define WIN_EXPORT_PREFIX WIN_SYM_PREFIX
#endif

// Intermediate macro to ensure the parameter is expanded before stringified.
#define STRINGIFY_(A) #A
#define STRINGIFY(A) STRINGIFY_(A)

#if !SANITIZER_GO

// ---------------------------------------------------------------------------
// MinGW / GCC native implementation.
//
// GCC supports genuine weak symbols (``__attribute__((weak))``) and weak
// aliases (``__attribute__((weak, alias("...")))``), so the MSVC
// ``/alternatename`` emulation is unnecessary.  ``WIN_FORCE_LINK`` becomes a
// ``__attribute__((used))`` strong reference that forces the linker to pull the
// defining archive member (GNU ld only includes an archive member when it
// satisfies an undefined strong reference).  ``WIN_EXPORT`` is a no-op for the
// static runtime (there is no DLL to export from).
// ---------------------------------------------------------------------------
#if defined(__MINGW32__) && defined(__GNUC__)

// No linker pragma is emitted for the static runtime.
#define WIN_WEAK_ALIAS(Name, Default)

// Force the linker to include the object defining ``Name`` by creating a strong
// reference that cannot be optimized away (``used``).  ``Name`` is declared as
// an opaque incomplete array so the macro does not need to know its real type;
// PE/COFF symbol resolution is by name, so the cross-TTU type difference is
// harmless at link time.
#define WIN_FORCE_LINK(Name)                                                   \
  extern "C" char Name[];                                                      \
  __attribute__((used)) static void *__asan_force_link_##Name = (void *)(Name);

// Static runtime: nothing to export.
#define WIN_EXPORT(ExportedName, Name)

// Dummy names retained for source compatibility with the upstream macros.
#define WEAK_DEFAULT_NAME(Name) Name##__def
#define WEAK_EXPORT_NAME(Name) Name##__dll

// Define and export a weak function.  ``Name`` is a weak alias of the default
// ``Name__def`` (whose body follows the macro invocation); a user may override
// it by providing a strong ``Name`` definition, exactly like on Linux.
#define WIN_WEAK_EXPORT_DEF(ReturnType, Name, ...)                             \
  extern "C" ReturnType WEAK_DEFAULT_NAME(Name)(__VA_ARGS__);                  \
  extern "C" ReturnType Name(__VA_ARGS__)                                      \
      __attribute__((                                                          \
          weak, alias(STRINGIFY(WEAK_DEFAULT_NAME(Name)))));                   \
  extern "C" ReturnType WEAK_DEFAULT_NAME(Name)(__VA_ARGS__)

// DLL-import side alias; unused for the static runtime.
#define WIN_WEAK_IMPORT_DEF(Name)

#else // !(defined(__MINGW32__) && defined(__GNUC__))

// ----------------- A workaround for the absence of weak symbols --------------
// We don't have a direct equivalent of weak symbols when using MSVC, but we can
// use the /alternatename directive to tell the linker to default a specific
// symbol to a specific value.
// Take into account that this is a pragma directive for the linker, so it will
// be ignored by the compiler and the function will be marked as UNDEF in the
// symbol table of the resulting object file. The linker won't find the default
// implementation until it links with that object file.
// So, suppose we provide a default implementation "fundef" for "fun", and this
// is compiled into the object file "test.obj" including the pragma directive.
// If we have some code with references to "fun" and we link that code with
// "test.obj", it will work because the linker always link object files.
// But, if "test.obj" is included in a static library, like "test.lib", then the
// liker will only link to "test.obj" if necessary. If we only included the
// definition of "fun", it won't link to "test.obj" (from test.lib) because
// "fun" appears as UNDEF, so it doesn't resolve the symbol "fun", and will
// result in a link error (the linker doesn't find the pragma directive).
// So, a workaround is to force linkage with the modules that include weak
// definitions, with the following macro: WIN_FORCE_LINK()

#define WIN_WEAK_ALIAS(Name, Default)                                          \
  __pragma(comment(linker, "/alternatename:" WIN_SYM_PREFIX STRINGIFY(Name) "="\
                                             WIN_SYM_PREFIX STRINGIFY(Default)))

#define WIN_FORCE_LINK(Name)                                                   \
  __pragma(comment(linker, "/include:" WIN_SYM_PREFIX STRINGIFY(Name)))

#define WIN_EXPORT(ExportedName, Name)                                         \
  __pragma(comment(linker, "/export:" WIN_EXPORT_PREFIX STRINGIFY(ExportedName)\
                                  "=" WIN_EXPORT_PREFIX STRINGIFY(Name)))

// We cannot define weak functions on Windows, but we can use WIN_WEAK_ALIAS()
// which defines an alias to a default implementation, and only works when
// linking statically.
// So, to define a weak function "fun", we define a default implementation with
// a different name "fun__def" and we create a "weak alias" fun = fun__def.
// Then, users can override it just defining "fun".
// We impose "extern "C"" because otherwise WIN_WEAK_ALIAS() will fail because
// of name mangling.

// Dummy name for default implementation of weak function.
# define WEAK_DEFAULT_NAME(Name) Name##__def
// Name for exported implementation of weak function.
# define WEAK_EXPORT_NAME(Name) Name##__dll

// Use this macro when you need to define and export a weak function from a
// library. For example:
//   WIN_WEAK_EXPORT_DEF(bool, compare, int a, int b) { return a > b; }
# define WIN_WEAK_EXPORT_DEF(ReturnType, Name, ...)                            \
  WIN_WEAK_ALIAS(Name, WEAK_DEFAULT_NAME(Name))                                \
  WIN_EXPORT(WEAK_EXPORT_NAME(Name), Name)                                     \
  extern "C" ReturnType Name(__VA_ARGS__);                                     \
  extern "C" ReturnType WEAK_DEFAULT_NAME(Name)(__VA_ARGS__)

// Use this macro when you need to import a weak function from a library. It
// defines a weak alias to the imported function from the dll. For example:
//   WIN_WEAK_IMPORT_DEF(compare)
# define WIN_WEAK_IMPORT_DEF(Name)                                             \
  WIN_WEAK_ALIAS(Name, WEAK_EXPORT_NAME(Name))

#endif // defined(__MINGW32__) && defined(__GNUC__)

#else // SANITIZER_GO

// Go neither needs nor wants weak references.
// The shenanigans above don't work for gcc.
# define WIN_WEAK_EXPORT_DEF(ReturnType, Name, ...)                            \
  extern "C" ReturnType Name(__VA_ARGS__)

#endif // SANITIZER_GO

#endif // SANITIZER_WINDOWS
#endif // SANITIZER_WIN_DEFS_H
