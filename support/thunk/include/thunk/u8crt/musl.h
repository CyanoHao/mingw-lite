#pragma once

#include <stdarg.h>
#include <stddef.h>
#include <sys/types.h>
#include <wchar.h>

namespace mingw_thunk
{
  namespace musl // multibyte
  {
    struct mbstate_t;

    size_t mbrtoc16(char16_t *pc16, const char *s, size_t n, mbstate_t *ps);
    size_t mbrtoc16(wchar_t *pc16, const char *s, size_t n, mbstate_t *ps);
    size_t mbrtoc32(char32_t *pc32, const char *s, size_t n, mbstate_t *ps);
    size_t mbrtowc(char32_t *wc, const char *s, size_t n, mbstate_t *ps);

    size_t c16rtomb(char *s, char16_t c16, mbstate_t *ps);
    size_t c16rtomb(char *s, wchar_t c16, mbstate_t *ps);
    size_t c32rtomb(char *s, char32_t c32, mbstate_t *ps);
  } // namespace musl

  namespace musl // env
  {
    extern char **__environ;

    char *getenv(const char *name);
    char *getenv(const char *name, size_t name_len);

    int setenv(const char *var, const char *value, int overwrite);
    int setenv(const char *var,
               size_t var_len,
               const char *value,
               size_t val_len,
               int overwrite);

    int unsetenv(const char *name);
    int unsetenv(const char *name, size_t name_len);
  } // namespace musl

  namespace musl // stdio
  {
    struct FILE;
    struct iovec;

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

    int fflush(FILE *f);
    int fgetc(FILE *f);
    char *fgets(char *s, int n, FILE *f);
    int fputc(int c, FILE *f);
    size_t fread(void *dest, size_t size, size_t nmemb, FILE *f);
    size_t fwrite(const void *src, size_t size, size_t nmemb, FILE *f);
    int ungetc(int c, FILE *f);
    int vfprintf(FILE *f, const char *fmt, va_list ap);
    int vfscanf(FILE *f, const char *fmt, va_list ap);
  } // namespace musl

  namespace musl // unistd
  {
    ssize_t read(int fd, void *buf, size_t count);
    ssize_t readv(int fd, const struct iovec *iov, int iovcnt);
    ssize_t write(int fd, const void *buf, size_t count);
    ssize_t writev(int fd, const struct iovec *iov, int iovcnt);
  } // namespace musl

  namespace musl // win32
  {
    extern char **utf8_argv;
  }
} // namespace mingw_thunk
