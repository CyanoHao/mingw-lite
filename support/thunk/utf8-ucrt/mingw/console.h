#pragma once

#include <stdint.h>
#include <stdio.h>

#include <io.h>
#include <windows.h>

namespace mingw_thunk::ucrt
{

  struct console
  {
    console() noexcept;
    console(int fd, HANDLE fh) noexcept;

    struct guard
    {
      guard() noexcept;
      guard(guard &&) noexcept;
      guard &operator=(guard &&) noexcept;

      guard(const guard &) = delete;
      guard &operator=(const guard &) = delete;

      ~guard() noexcept;

    private:
      guard(console *c) noexcept;

      console *c;

      friend struct console;
    };

    void acquire() noexcept;
    guard acquire_guard() noexcept;
    void release() noexcept;

    int putc(unsigned char c) noexcept;
    int putc(wchar_t c) noexcept;
    int putc_nolock(unsigned char c) noexcept;
    int putc_nolock(wchar_t c) noexcept;

    size_t write_nolock(void const *p, size_t n) noexcept;

    void flush() noexcept;
    void flush_nolock() noexcept;

    void open(int fd) noexcept;
    void close() noexcept;

    static bool is_console(int fd) noexcept;
    static bool is_console(FILE *fp) noexcept;

    static console *get(int fd, bool allocate) noexcept;
    static console *get(FILE *fp, bool allocate) noexcept;
    static void flush_all() noexcept;

  private:
    constexpr static int wbuf_size = 1024;
    constexpr static int wbuf_flush_threshold = 1000;

    bool lock;
    int fd;
    HANDLE fh;
    unsigned char tail[4];
    int tail_len;
    wchar_t whigh;
    wchar_t wbuf[wbuf_size];
    int wlen;

    static console standard_instance[3];
    static console **dynamic_instance;
    static int dynamic_size;
    static bool dynamic_lock;

    constexpr static int dynamic_inc = 16;

    void *operator new(size_t, void *ptr) noexcept;

    void bind_nolock(int fd) noexcept;
    void bind_nolock(int fd, HANDLE fh) noexcept;
    void reset_nolock() noexcept;

    bool emit_nolock(char32_t cp) noexcept;
    void drop_tail_nolock(int n) noexcept;

    static void write_console(HANDLE fh, wchar_t const *buf, int len) noexcept;

    static bool is_stdout(int fd) noexcept;
    static bool is_stderr(int fd) noexcept;
    static bool standard_stream_should_flush(int fd, char32_t cp) noexcept;
  };

  bool is_console(int fd) noexcept;
  bool is_console(FILE *fp) noexcept;

  void console_open(int fd) noexcept;
  void console_close(int fd) noexcept;

  void __attribute__((used, destructor)) flush_on_exit() noexcept;

} // namespace mingw_thunk::ucrt
