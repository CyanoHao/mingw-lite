#pragma once

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

namespace mingw_thunk::ucrt
{

  int __cdecl __stdio_common_vfprintf(uint64_t options,
                                      FILE *stream,
                                      const char *format,
                                      _locale_t locale,
                                      va_list arglist);
  int __cdecl __stdio_common_vfwprintf(uint64_t options,
                                       FILE *stream,
                                       const wchar_t *format,
                                       _locale_t locale,
                                       va_list arglist);

  int __cdecl __stdio_common_vfprintf_s(uint64_t options,
                                        FILE *stream,
                                        const char *format,
                                        _locale_t locale,
                                        va_list arglist);
  int __cdecl __stdio_common_vfwprintf_s(uint64_t options,
                                         FILE *stream,
                                         const wchar_t *format,
                                         _locale_t locale,
                                         va_list arglist);

  int __cdecl __stdio_common_vfprintf_p(uint64_t options,
                                        FILE *stream,
                                        const char *format,
                                        _locale_t locale,
                                        va_list arglist);
  int __cdecl __stdio_common_vfwprintf_p(uint64_t options,
                                         FILE *stream,
                                         const wchar_t *format,
                                         _locale_t locale,
                                         va_list arglist);

  int __cdecl __stdio_common_vsprintf(uint64_t options,
                                      char *str,
                                      size_t len,
                                      const char *format,
                                      _locale_t locale,
                                      va_list arglist);
  int __cdecl __stdio_common_vswprintf(uint64_t options,
                                       wchar_t *str,
                                       size_t len,
                                       const wchar_t *format,
                                       _locale_t locale,
                                       va_list arglist);

  int __cdecl __stdio_common_vsprintf_s(uint64_t options,
                                        char *str,
                                        size_t count,
                                        const char *format,
                                        _locale_t locale,
                                        va_list arglist);
  int __cdecl __stdio_common_vswprintf_s(uint64_t options,
                                         wchar_t *str,
                                         size_t count,
                                         const wchar_t *format,
                                         _locale_t locale,
                                         va_list arglist);

  int __cdecl __stdio_common_vsnprintf_s(uint64_t options,
                                         char *str,
                                         size_t size,
                                         size_t maxcount,
                                         const char *format,
                                         _locale_t locale,
                                         va_list arglist);
  int __cdecl __stdio_common_vsnwprintf_s(uint64_t options,
                                          wchar_t *str,
                                          size_t size,
                                          size_t maxcount,
                                          const wchar_t *format,
                                          _locale_t locale,
                                          va_list arglist);

  int __cdecl __stdio_common_vsprintf_p(uint64_t options,
                                        char *str,
                                        size_t count,
                                        const char *format,
                                        _locale_t locale,
                                        va_list arglist);
  int __cdecl __stdio_common_vswprintf_p(uint64_t options,
                                         wchar_t *str,
                                         size_t count,
                                         const wchar_t *format,
                                         _locale_t locale,
                                         va_list arglist);

  int fflush(FILE *stream);

  int fputc(int c, FILE *stream);
  int fputs(const char *s, FILE *stream);
  size_t fwrite(const void *ptr, size_t size, size_t count, FILE *stream);
  int puts(const char *s);

  int _write(int fd, const void *buffer, unsigned int count);

} // namespace mingw_thunk::ucrt

namespace mingw_thunk::ucrt
{

  bool is_console(int fd) noexcept;
  bool is_console(FILE *fp) noexcept;

  void console_open(int fd) noexcept;
  void console_close(int fd) noexcept;

} // namespace mingw_thunk::ucrt
