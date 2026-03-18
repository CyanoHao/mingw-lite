#pragma once

#include <stdarg.h>

namespace mingw_thunk
{
  namespace musl
  {
    struct FILE;

    extern struct FILE *g_stdin;
    extern struct FILE *g_stdout;
    extern struct FILE *g_stderr;

    inline FILE *g_fp_from_fd(int fd)
    {
      switch (fd) {
      case 0:
        return g_stdin;
      case 1:
        return g_stdout;
      case 2:
        return g_stderr;
      default:
        return nullptr;
      }
    }

    int vfprintf(FILE *f, const char *fmt, va_list ap);
    int vfscanf(FILE *f, const char *fmt, va_list ap);
  } // namespace musl
} // namespace mingw_thunk
