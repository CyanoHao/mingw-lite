#pragma once

#include <stdarg.h>
#include <stddef.h>
#include <sys/types.h>
#include <time.h>
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

    /* Console channel pool for fd >= 3 (utf8-musl/win32/console_channel.cc):
     * returns the slot's FILE, or a degraded sink (reads EOF / writes
     * F_ERR) when the pool is exhausted or the fd has no handle. */
    FILE *console_channel_fp(int fd) noexcept;

    /* Flush and drop the channel owned by fd (call while the fd is still
     * open); fd 0/1/2 only flush the static trio.  fflush(nullptr)
     * support flushes every occupied channel.  on_open drops any channel
     * keyed to a freshly (re)assigned fd (deterministic fd-reuse guard
     * for the open-family thunks). */
    void console_channel_release(int fd) noexcept;
    void console_channel_flush_all() noexcept;
    void console_channel_on_open(int fd) noexcept;

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
        return console_channel_fp(fd);
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

    /* Memory channels (string-file shapes ported from musl upstream);
     * vsnprintf returns the required length even when truncated. */
    int vsnprintf(char *s, size_t n, const char *fmt, va_list ap);
    int vsscanf(const char *s, const char *fmt, va_list ap);

    /* Stream-channel bridges over a native FILE* (utf8-musl/win32/
     * native_bridge.cc): the write side renders through the engine and
     * writes back to the SAME native FILE (its buffering, position and
     * interleaving stay authoritative); the read side pulls bytes via
     * _fgetc_nolock under _lock_file and pushes back at most one
     * lookahead byte via ungetc on teardown. */
    int vfprintf_to_native(void *native_fp, const char *fmt, va_list ap);
    int vfscanf_from_native(void *native_fp, const char *fmt, va_list ap);

    /* Minimum exponent digits for %e/%f/%g (engine-side TLS state; the
     * printf shell sets it per call from the options argument: 2 default,
     * 3 for the three-digit-exponent option 0x10). */
    extern __thread int exp_digits_min;
  } // namespace musl

  namespace musl // stdlib
  {
    /* strtox engines (utf8-musl/stdlib/strtod.cc): musl memory-FILE
     * wrappers over __floatscan(pok=1); the decimal point is always '.'
     * (frozen-C engine doctrine — the native family follows LC_NUMERIC,
     * which is exactly why this layer owns it).  prec: strtof=0 (FLT),
     * strtod=1 (DBL), strtold=2 (LDBL, true 80-bit on i686).  The engine
     * sets errno=ERANGE on overflow / inexact-underflow (native shape)
     * and errno=EINVAL on a no-digit prefix (native leaves errno alone —
     * the thunks mask that one). */
    double strtod(const char *s, char **p);
    float strtof(const char *s, char **p);
    long double strtold(const char *s, char **p);
  } // namespace musl

  namespace musl // unistd
  {
    ssize_t read(int fd, void *buf, size_t count);
    ssize_t readv(int fd, const struct iovec *iov, int iovcnt);
    ssize_t write(int fd, const void *buf, size_t count);
    ssize_t writev(int fd, const struct iovec *iov, int iovcnt);
  } // namespace musl

  namespace musl // time
  {
    /* strftime/wcsftime engines (utf8-musl/time/): musl ports with the
     * C.UTF-8 English name tables; %z/%Z/%s read the tz state below
     * (the Windows struct tm has no tm_gmtoff/tm_zone). */
    size_t strftime(char *s, size_t n, const char *f, const struct tm *tm);
    size_t
    wcsftime(wchar_t *s, size_t n, const wchar_t *f, const struct tm *tm);

    /* C.UTF-8 timezone state (utf8-musl/internal/tz_state.cc, plan-2
     * M9): TZ environment variable (POSIX subset) wins, else
     * GetTimeZoneInformation.  The UCRT tz globals and the strftime
     * %z/%Z/%s specifiers all read THIS state — one truth source,
     * never the drift-prone native locale machinery. */
    namespace tz
    {
      struct state
      {
        long timezone; /* seconds, positive WEST of UTC (UCRT) */
        long dstbias;  /* seconds, negative while DST is active */
        int daylight;  /* nonzero when the zone observes DST */
        char std_name[64];
        char dlt_name[64];
        char *tzname[2]; /* { std_name, dlt_name } — stable */
      };

      state &get() noexcept;  /* lazy init on first use */
      void reinit() noexcept; /* _tzset: re-read TZ / registry */

      /* UTC offset in seconds, positive EAST (strftime %z/%s math) */
      long utc_offset(int isdst) noexcept;
    } // namespace tz
  } // namespace musl

  namespace musl // win32
  {
    extern char **utf8_argv;
  }

  namespace musl_ucrt
  {
    bool is_console(::FILE *fp);
    bool is_console(int fd);
  } // namespace musl_ucrt

} // namespace mingw_thunk
