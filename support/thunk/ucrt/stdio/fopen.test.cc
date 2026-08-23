#include <catch_amalgamated.hpp>

#include <direct.h>
#include <errno.h>
#include <io.h>
#include <sys/stat.h>

// Direct symbol reference binds the overlay thunk (stdio_shell.test.cc
// precedent); int matches errno_t on the ABI level.
#ifndef _UCRT
extern "C" int __cdecl fopen_s(FILE **pFile, const char *filename, const char *mode);
#endif

TEST_CASE("fopen")
{
  const char *paths[] = {
      "test-fopen-你好.txt",
      "test-fopen-こんにちは.txt",
      "test-fopen-안녕하세요.txt",
      "test-fopen-👋🌏.txt",
  };

  for (auto path : paths) {
    FILE *fp = fopen(path, "wb+");
    REQUIRE(fp);
    fclose(fp);
  }
}

TEST_CASE("fopen_s")
{
  // success: ret 0 + UTF-8 content round trip through the forwarded
  // fopen thunk (path transcode + channel probe fully shared); the
  // console path (CONOUT$) is inherited from that thunk and covered by
  // console_channel.test.cc — not re-asserted headless here
  FILE *fp = (FILE *)1;
  REQUIRE(fopen_s(&fp, "test-fopen_s-你好.txt", "wb+") == 0);
  REQUIRE(fp != nullptr);
  const char ni[] = "\xe4\xbd\xa0"; // 你
  REQUIRE(fwrite(ni, 1, sizeof ni, fp) == sizeof ni);
  rewind(fp);
  char buf[8];
  REQUIRE(fread(buf, 1, sizeof ni, fp) == sizeof ni);
  REQUIRE(memcmp(buf, ni, sizeof ni) == 0);
  REQUIRE(fclose(fp) == 0);

  // missing file: ret == errno code (wine shape: ENOENT) + nulled out
  fp = (FILE *)1;
  errno = 0;
  REQUIRE(fopen_s(&fp, "no-such-u8crt-file.bin", "rb") == ENOENT);
  REQUIRE(errno == ENOENT);
  REQUIRE(fp == nullptr);

  // null pFile: EINVAL, no crash (wine shape)
  REQUIRE(fopen_s(nullptr, "x", "w") == EINVAL);

  // null filename / null mode: EINVAL with *pFile pre-nulled
  fp = (FILE *)1;
  REQUIRE(fopen_s(&fp, nullptr, "w") == EINVAL);
  REQUIRE(fp == nullptr);
  fp = (FILE *)1;
  REQUIRE(fopen_s(&fp, "x", nullptr) == EINVAL);
  REQUIRE(fp == nullptr);
}
