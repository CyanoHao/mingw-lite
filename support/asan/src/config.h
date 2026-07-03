/* config.h for MinGW-lite libasan (vendored from GCC 15.3.0 libsanitizer).
 *
 * NOTE: the sanitizer runtime sources do not include <config.h>; this file is
 * retained only as a fallback / documentation of the build configuration.  The
 * xmake build does NOT pass -DHAVE_CONFIG_H.  All SANITIZER_* feature macros
 * are derived by sanitizer_platform.h from _WIN32/_WIN64.  libbacktrace is not
 * built, so the libbacktrace/backtrace-rename.h include from the autoconf build
 * is intentionally absent. */
#ifndef LIBSANITIZER_CONFIG_H
#define LIBSANITIZER_CONFIG_H

/* Pointer width; sanitizer_platform.h also derives SANITIZER_WORDSIZE from
 * _WIN64, but keep this in sync for anything that probes it directly. */
#ifdef _WIN64
#define SIZEOF_VOID_P 8
#else
#define SIZEOF_VOID_P 4
#endif

#define HAVE_SYNC_FUNCTIONS 1
#define HAVE_ATOMIC_FUNCTIONS 1
#define HAVE_DECL_STRNLEN 1

#define BACKTRACE_ELF_SIZE /* not ELF; libbacktrace not built */

#define STDC_HEADERS 1
#define HAVE_STDINT_H 1
#define HAVE_INTTYPES_H 1
#define HAVE_STRING_H 1
#define HAVE_MEMORY_H 1
#define HAVE_STDLIB_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_UNISTD_H 1

#define PACKAGE "package-unused"
#define PACKAGE_VERSION "version-unused"

#endif /* LIBSANITIZER_CONFIG_H */
