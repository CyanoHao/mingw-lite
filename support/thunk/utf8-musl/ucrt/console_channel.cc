#include "console_channel.h"

#include <io.h>
#include <stdio.h>

#include <thunk/u8crt/musl.h>

namespace mingw_thunk
{
  namespace musl
  {
    namespace
    {
      enum
      {
        kFree = 0,
        kInit = 1,
        kReady = 2
      };

      constexpr int kChannelSlots = 8;

      console_channel g_channels[kChannelSlots];

      /* Degraded sink handed out when the pool is exhausted: reads see
       * EOF and writes raise F_ERR, so printf stops emitting instead of
       * silently dropping output. */
      size_t sink_write(FILE *f, const unsigned char *, size_t) noexcept
      {
        f->flags |= F_ERR;
        return 0;
      }

      size_t sink_read(FILE *f, unsigned char *, size_t) noexcept
      {
        f->flags |= F_EOF;
        return 0;
      }

      FILE g_sink_file = {
          .read = &sink_read,
          .write = &sink_write,
          .lock = -1,
          .lbf = -1,
      };

      void init_channel(console_channel *ch, int fd, HANDLE h) noexcept
      {
        ch->file = {};
        ch->file.read = __stdio_read;
        ch->file.write = __stdio_write;
        ch->file.seek = __console_seek;
        ch->file.close = __console_close;
        ch->file.buf = ch->buf + UNGET;
        ch->file.buf_size = sizeof ch->buf - UNGET;
        ch->file.fd = fd;
        ch->file.lock = -1;
        ch->file.lbf = '\n';
        ch->tail = {};
        ch->handle = h;
        ch->fd = fd;
      }

      void flush_channel_locked(FILE *f) noexcept
      {
        if (f->wpos != f->wbase)
          f->write(f, 0, 0);
      }

      void clear_channel(console_channel *ch) noexcept
      {
        /* keep the FILE lock byte held by the caller while the rest of
         * the slot is wiped, so concurrent validators that grab the lock
         * mid-wipe always observe fd == -1 / write == nullptr and bail */
        volatile int &lock = ch->file.lock;
        int held = lock;
        ch->file = {};
        ch->tail = {};
        ch->handle = nullptr;
        ch->fd = -1;
        lock = held;
      }

      /* Drop a slot whose fd/handle pair is stale (fd reuse).  The exact
       * handle is re-validated under the FILE lock; buffered data is
       * never flushed — it belongs to a dead fd. */
      bool reset_stale(console_channel *ch, int fd, HANDLE stale) noexcept
      {
        FLOCK(&ch->file);
        bool ours = __atomic_load_n(&ch->state, __ATOMIC_ACQUIRE) == kReady &&
                    ch->fd == fd && ch->handle == stale;
        if (ours) {
          clear_channel(ch);
          __atomic_store_n(&ch->state, kFree, __ATOMIC_RELEASE);
        }
        FUNLOCK(&ch->file);
        return ours;
      }

      /* Validate, optionally flush (when the fd still carries the
       * acquired handle), then drop the slot.  The state stays kReady
       * for the whole flush so the flush's own tail lookups hit this
       * slot instead of acquiring a duplicate channel. */
      void close_slot(console_channel *ch, int fd, bool flush) noexcept
      {
        FLOCK(&ch->file);
        if (__atomic_load_n(&ch->state, __ATOMIC_ACQUIRE) == kReady &&
            ch->fd == fd) {
          if (flush) {
            intptr_t h = _get_osfhandle(fd);
            if (h != -1 && (HANDLE)h == ch->handle)
              flush_channel_locked(&ch->file);
          }
          clear_channel(ch);
          __atomic_store_n(&ch->state, kFree, __ATOMIC_RELEASE);
        }
        FUNLOCK(&ch->file);
      }
    } // namespace

    console_channel *console_channel_for_fd(int fd) noexcept
    {
      if (fd < 3)
        return nullptr; /* the static trio owns fd 0/1/2 */

      intptr_t h = _get_osfhandle(fd);
      if (h == -1)
        return nullptr;

      /* pass 1: published hit */
      for (console_channel &ch : g_channels) {
        if (__atomic_load_n(&ch.state, __ATOMIC_ACQUIRE) != kReady)
          continue;
        if (ch.fd == fd && ch.handle == (HANDLE)h)
          return &ch;
      }

      /* pass 2: reset stale slots (fd closed and reused) */
      for (console_channel &ch : g_channels) {
        if (__atomic_load_n(&ch.state, __ATOMIC_ACQUIRE) != kReady)
          continue;
        if (ch.fd == fd && ch.handle != (HANDLE)h)
          reset_stale(&ch, fd, ch.handle);
      }

      /* pass 3: acquire a free slot */
      console_channel *mine = nullptr;
      for (console_channel &ch : g_channels) {
        int expected = kFree;
        if (__atomic_compare_exchange_n(&ch.state, &expected, kInit, false,
                                         __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)) {
          init_channel(&ch, fd, (HANDLE)h);
          __atomic_store_n(&ch.state, kReady, __ATOMIC_RELEASE);
          mine = &ch;
          break;
        }
      }
      if (!mine)
        return nullptr; /* exhausted */

      /* pass 4: a concurrent acquirer of the same fd may have published
       * first; the later publisher defers to it so a channel is never
       * duplicated (split tails would corrupt partial sequences). */
      for (console_channel &ch : g_channels) {
        if (&ch == mine)
          continue;
        if (__atomic_load_n(&ch.state, __ATOMIC_ACQUIRE) != kReady)
          continue;
        if (ch.fd == fd && ch.handle == (HANDLE)h) {
          reset_stale(mine, fd, mine->handle);
          return &ch;
        }
      }

      return mine;
    }

    FILE *console_channel_fp(int fd) noexcept
    {
      console_channel *ch = console_channel_for_fd(fd);
      return ch ? &ch->file : &g_sink_file;
    }

    utf8_buffer *console_channel_tail(int fd) noexcept
    {
      console_channel *ch = console_channel_for_fd(fd);
      return ch ? &ch->tail : nullptr;
    }

    void console_channel_release(int fd) noexcept
    {
      if (fd == 1) {
        fflush(g_stdout); /* static trio: flush only, never released */
        return;
      }
      if (fd == 2) {
        fflush(g_stderr);
        return;
      }
      if (fd == 0)
        return; /* read side: nothing to flush */

      for (console_channel &ch : g_channels) {
        if (__atomic_load_n(&ch.state, __ATOMIC_ACQUIRE) != kReady)
          continue;
        if (ch.fd == fd) {
          close_slot(&ch, fd, true);
          return;
        }
      }
    }

    void console_channel_on_open(int fd) noexcept
    {
      if (fd < 3)
        return;

      for (console_channel &ch : g_channels) {
        if (__atomic_load_n(&ch.state, __ATOMIC_ACQUIRE) != kReady)
          continue;
        if (ch.fd == fd)
          close_slot(&ch, fd, false); /* stale by construction: no flush */
      }
    }

    void console_channel_flush_all() noexcept
    {
      if (g_stdout->wpos != g_stdout->wbase)
        fflush(g_stdout);
      if (g_stderr->wpos != g_stderr->wbase)
        fflush(g_stderr);

      for (console_channel &ch : g_channels) {
        if (__atomic_load_n(&ch.state, __ATOMIC_ACQUIRE) != kReady)
          continue;
        FLOCK(&ch.file);
        if (ch.file.write) /* still initialized (not reset mid-flight) */
          flush_channel_locked(&ch.file);
        FUNLOCK(&ch.file);
      }
    }
  } // namespace musl
} // namespace mingw_thunk
