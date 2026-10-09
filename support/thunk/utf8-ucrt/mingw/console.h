#pragma once

// UTF-8 console state for the ported stdio engine (console.cc).
//
// This is the write-side sink _fputc_nolock_internal hands UTF-8 bytes
// to when a stream's fd is a console: bytes are reassembled into code
// points, converted to UTF-16 and emitted through WriteConsoleW -- the
// same transcode the native CRT performs in ucrtbase's
// write_double_translated_ansi_nolock (lowio/write.cpp), except our
// narrow encoding is UTF-8 by doctrine instead of the console/ANSI
// codepage, so no information is lost on non-UTF-8 consoles.
//
// Everything here is built from public CRT entry points only (_fileno,
// _get_osfhandle) plus Win32 (GetConsoleMode, WriteConsoleW); the
// native FILE object is never touched on this path.
//
// State encoding -- there is no separate verdict field; the (fd, fh)
// pair carries everything, and fh is always the handle snapshot the
// verdict was made at:
//
//   fd == -1, fh == INVALID_HANDLE_VALUE  virgin / closed; a dynamic
//                                         slot in this state is free
//                                         for reuse
//   fd == -1, fh == h                     negative cache: judged "not
//                                         a console" at snapshot h
//   fd >= 0,  fh == h                     bound console channel
//
// Whenever _get_osfhandle(fd) differs from fh the state is stale and
// the next use re-probes once, so the console verdict only costs a
// GetConsoleMode when the snapshot actually changed (freopen / dup2 /
// fd recycle), never per byte.  The static trio (fds 0/1/2) keeps its
// negative cache forever; dynamic negative entries double as free
// slots, so a file fd never permanently occupies registry memory.
//
// Errors are reported through plain errno (never ptd); the stdio
// engine propagates them into its ptd cache.  get(fd, true) returns
// nullptr on allocation failure only -- every other fd resolves to
// some object, files included (as a negative cache), and the caller
// falls back to the original CRT for those bytes.

#include <stdio.h>

#include <io.h>
#include <windows.h>

namespace mingw_thunk::ucrt
{

  struct console
  {
    enum class result
    {
      ok,          /* consumed; a parked partial sequence counts as
                    * written (native _mbBuffer parity) */
      not_console, /* the fd is not (or is no longer) a console: the
                    * caller falls back to the native narrow path for
                    * this byte */
      error        /* WriteConsoleW failed; errno is set */
    };

    console() noexcept;
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

    /* flush() takes the object lock; the _nolock variants require it
     * held (put/emit paths, close, the buffering guard). */
    void flush() noexcept;
    void flush_nolock() noexcept;
    void flush_stdout_or_stderr() noexcept;
    void flush_stdout_or_stderr_nolock() noexcept;

    /* consume one UTF-8 byte; requires the object lock held and this
     * object to be the one get() resolved for fd.  fd is passed in
     * because a negative-cache entry no longer remembers its number.
     * Flushes only per the buffer policy (full buffer, stdout LF,
     * stderr per character); upper layers flush explicitly. */
    result put(int fd, unsigned char c) noexcept;

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

    /* lock-held cores shared by the public entry points */
    void bind_nolock(int fd, HANDLE h, bool is_console) noexcept;
    result put_nolock(unsigned char c) noexcept;
    bool emit_nolock(char32_t cp) noexcept;
    bool write_nolock() noexcept;
    void drop_tail_nolock(int n) noexcept;
  };

} // namespace mingw_thunk::ucrt
