#pragma once

#include <stdio.h>

#include <io.h>
#include <windows.h>

namespace mingw_thunk::ucrt
{

  struct console
  {
    console() noexcept;
    console(int fd) noexcept;
    console(int fd, HANDLE fh) noexcept;

    struct guard
    {
      ~guard() noexcept;

    private:
      guard(console *c) noexcept;

      console *c;

      friend struct console;
    };

    void acquire() noexcept;
    guard acquire_guard() noexcept;
    void release() noexcept;

    void flush() noexcept;
    void flush_nolock() noexcept;
    void flush_stdout_or_stderr() noexcept;
    void flush_stdout_or_stderr_nolock() noexcept;

    void open(int fd) noexcept;
    void close() noexcept;
    void reset() noexcept;
    void reset_nolock() noexcept;

    static console *get(int fd, bool allocate) noexcept;
    static console *get(FILE *fp, bool allocate) noexcept;

  private:
    bool lock;
    int fd;
    HANDLE fh;
    unsigned char tail[4];
    int tail_len;
    wchar_t wbuf[1024];
    int wlen;

    static console standard_instance[3];
    static console **dynamic_instance;
    static int dynamic_size;
    static bool dynamic_lock;

    constexpr static int dynamic_inc = 16;

    void *operator new(size_t, void *ptr) noexcept;
  };

} // namespace mingw_thunk::ucrt
