#include <catch_amalgamated.hpp>

#include <fcntl.h>
#include <io.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/stat.h>
#include <windows.h>

// Channel-layer tests (musl layer, headless): the slot pool is pure
// bookkeeping and never calls GetConsoleMode, so it is driven here with
// ordinary file fds.  The IAT-level shells arrive with M1-b/M2; the
// console end-to-end batch is test/console.c under wineconsole.
#include "../../utf8-musl/internal/stdio_impl.h"
#include "../../utf8-musl/win32/console_channel.h"
#include <thunk/u8crt/musl.h>

namespace
{
  namespace musl = mingw_thunk::musl;

  // native revert import (def alias), used to bypass our _close thunk in
  // the fd-reuse test the way an unhooked close would
  extern "C" __attribute__((dllimport)) int __cdecl __ms__close(int fd);

  int open_temp()
  {
    return _open("test-console-channel.tmp",
                 _O_RDWR | _O_CREAT | _O_BINARY | _O_TRUNC,
                 _S_IREAD | _S_IWRITE);
  }
} // namespace

TEST_CASE("console channel static trio routing")
{
  REQUIRE(musl::g_fp_from_fd(0) == musl::g_stdin);
  REQUIRE(musl::g_fp_from_fd(1) == musl::g_stdout);
  REQUIRE(musl::g_fp_from_fd(2) == musl::g_stderr);
}

TEST_CASE("console channel slot acquire and lookup")
{
  int fd = open_temp();
  REQUIRE(fd >= 3);

  musl::console_channel *ch = musl::console_channel_for_fd(fd);
  REQUIRE(ch != nullptr);
  REQUIRE(musl::console_channel_for_fd(fd) == ch);

  // slot FILE mirrors the stdout.cc designated-initializer shape
  musl::FILE *f = &ch->file;
  REQUIRE(f->fd == fd);
  REQUIRE(f->lbf == '\n');
  REQUIRE(f->read == &musl::__stdio_read);
  REQUIRE(f->write == &musl::__stdio_write);
  REQUIRE(f->buf == ch->buf + musl::UNGET);
  REQUIRE(f->buf_size == sizeof ch->buf - musl::UNGET);
  REQUIRE(musl::g_fp_from_fd(fd) == f);

  musl::console_channel_release(fd);
  _close(fd);
}

TEST_CASE("console channel release frees the slot for reuse")
{
  int fd = open_temp();
  REQUIRE(fd >= 3);

  musl::console_channel *ch = musl::console_channel_for_fd(fd);
  REQUIRE(ch != nullptr);

  // the fclose/_close thunks must drop pending tails on release
  ch->tail.len = 2;
  ch->tail.buf[0] = 0xf0;

  musl::console_channel_release(fd);
  REQUIRE(ch->tail.len == 0);
  REQUIRE(__atomic_load_n(&ch->state, __ATOMIC_ACQUIRE) == 0); // kFree

  musl::console_channel *again = musl::console_channel_for_fd(fd);
  REQUIRE(again != nullptr);
  REQUIRE(again->fd == fd);

  musl::console_channel_release(fd);
  _close(fd);
}

TEST_CASE("console channel fd reuse resets the stale slot")
{
  int fd = open_temp();
  REQUIRE(fd >= 3);

  musl::console_channel *ch = musl::console_channel_for_fd(fd);
  REQUIRE(ch != nullptr);

  // simulate a pending partial sequence, then close the fd behind the
  // channel's back (no thunk) and reopen: the same fd number comes
  // back, possibly even with the same handle value
  ch->tail.len = 2;
  ch->tail.buf[0] = 0xf0;
  ch->tail.buf[1] = 0x9f;

  __ms__close(fd); // behind the channel's back (no thunk)

  int fd2 = open_temp();
  REQUIRE(fd2 >= 3);
  REQUIRE(fd2 == fd); // CRT fd table hands the lowest free number back

  // the open-family thunks report the (re)assignment; any channel still
  // keyed to the fd is stale by construction
  musl::console_channel_on_open(fd2);

  musl::console_channel *fresh_ch = musl::console_channel_for_fd(fd2);
  REQUIRE(fresh_ch != nullptr);
  // no dirty tail survives the fd reuse
  REQUIRE(fresh_ch->tail.len == 0);
  REQUIRE(fresh_ch->handle == (HANDLE)(intptr_t)_get_osfhandle(fd2));

  musl::console_channel_release(fd2);
  _close(fd2);
}

TEST_CASE("console channel pool exhaustion degrades to the sink")
{
  constexpr int n = 12; // pool holds 8 slots
  int fds[n];
  for (int &fd : fds) {
    fd = open_temp();
    REQUIRE(fd >= 3);
  }

  musl::FILE *fp[n];
  for (int i = 0; i < n; i++)
    fp[i] = musl::g_fp_from_fd(fds[i]);

  // the overflowing tail of the batch shares one degraded sink
  REQUIRE(fp[n - 1] == fp[n - 2]);
  REQUIRE(fp[0] != fp[n - 1]);
  REQUIRE(musl::fwrite("x", 1, 1, fp[n - 1]) == 0);
  REQUIRE(musl::ferror(fp[n - 1]));

  char buf[8];
  REQUIRE(musl::fgets(buf, sizeof buf, fp[n - 1]) == nullptr);

  for (int fd : fds) {
    musl::console_channel_release(fd);
    _close(fd);
  }
}

TEST_CASE("console channel lifecycle via fopen/fclose thunks")
{
  FILE *fp = fopen("test-console-channel-thunk.tmp", "wb+");
  REQUIRE(fp);
  int fd = _fileno(fp);
  REQUIRE(fd >= 3);

  // fopen reported the fresh fd; acquiring now yields a clean slot
  musl::console_channel *ch = musl::console_channel_for_fd(fd);
  REQUIRE(ch != nullptr);
  ch->tail.len = 1;
  ch->tail.buf[0] = 0xf0;

  // fclose must flush-and-drop the channel before the native close
  REQUIRE(fclose(fp) == 0);
  REQUIRE(__atomic_load_n(&ch->state, __ATOMIC_ACQUIRE) == 0); // kFree
  REQUIRE(ch->tail.len == 0);
}

TEST_CASE("utf8 buffer map routes fd to the slot tail")
{
  REQUIRE(&musl::g_utf8_buffer[0] != &musl::g_utf8_buffer[1]);
  REQUIRE(&musl::g_utf8_buffer[1] != &musl::g_utf8_buffer[2]);
  REQUIRE(&musl::g_utf8_buffer[0] == &musl::g_utf8_buffer[0]);

  int fd = open_temp();
  REQUIRE(fd >= 3);

  musl::g_utf8_buffer[fd].len = 3; // implies channel acquisition
  musl::console_channel *ch = musl::console_channel_for_fd(fd);
  REQUIRE(ch != nullptr);
  REQUIRE(ch->tail.len == 3);
  REQUIRE(&musl::g_utf8_buffer[fd] == &ch->tail);

  musl::console_channel_release(fd);
  REQUIRE(ch->tail.len == 0);

  // after release the mapping re-acquires a fresh tail
  REQUIRE(musl::g_utf8_buffer[fd].len == 0);
  musl::console_channel_release(fd);
  _close(fd);
}
