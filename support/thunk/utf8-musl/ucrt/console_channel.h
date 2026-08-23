#pragma once

#include "utf8_buffer.h"

#include "../internal/stdio_impl.h"

namespace mingw_thunk
{
  namespace musl
  {
    /* Console channel for fd >= 3 (fd 0/1/2 stay with the static
     * stdin/stdout/stderr trio and never occupy a slot).
     *
     * A slot is acquired lazily on first use and re-validated on every
     * lookup: _get_osfhandle(fd) must still match the handle snapshot
     * taken at acquisition.  A mismatch means the fd number was closed
     * and reused, so the whole slot (including a dirty UTF-8 tail) is
     * reset.  Slot state transitions use the tri-state `state` field;
     * slot contents are serialized by the musl FILE lock, which stays
     * held across flushes so the flush's own tail lookups keep hitting
     * this slot (never acquiring a duplicate). */
    struct console_channel
    {
      volatile int state;                /* 0 free, 1 init, 2 ready */
      int fd;                            /* valid once ready */
      HANDLE handle;                     /* _get_osfhandle(fd) snapshot */
      FILE file;
      unsigned char buf[BUFSIZ + UNGET]; /* file.buf = buf + UNGET */
      utf8_buffer tail;                  /* u8 remainder, slot lifetime */
    };

    /* Slot pool bookkeeping only; never calls GetConsoleMode, so it can
     * be driven with ordinary file fds in tests.  Nullptr when the pool
     * is exhausted or the fd has no handle. */
    console_channel *console_channel_for_fd(int fd) noexcept;

    /* Flush and drop the slot owned by fd (must run while the fd is
     * still open so the flush can reach the console).  fd 0/1/2 only
     * flush the static trio and are never released. */
    void console_channel_release(int fd) noexcept;

    /* An open/dup just (re)assigned this fd: any channel still keyed to
     * it is by construction stale (its fd was closed behind our back)
     * and is dropped without flushing — the buffer belongs to a dead
     * stream.  Called from the open-family thunks after a successful
     * return; this is the deterministic fd-reuse guard (the handle
     * comparison in for_fd only catches reuse when handle values
     * differ, which the kernel allocator does not guarantee). */
    void console_channel_on_open(int fd) noexcept;

    /* fflush(nullptr) support: flush g_stdout, g_stderr and every
     * occupied slot. */
    void console_channel_flush_all() noexcept;

    /* Tail lookup used by the g_utf8_buffer map (implies acquisition). */
    utf8_buffer *console_channel_tail(int fd) noexcept;
  } // namespace musl
} // namespace mingw_thunk
