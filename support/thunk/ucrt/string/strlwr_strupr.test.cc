#include <catch_amalgamated.hpp>

#include <errno.h>
#include <locale.h>
#include <string.h>

extern "C" char *__cdecl _strlwr(char *);
extern "C" char *__cdecl _strupr(char *);
typedef int errno_t;
extern "C" errno_t __cdecl _strlwr_s(char *, size_t);
extern "C" errno_t __cdecl _strupr_s(char *, size_t);
extern "C" char *__cdecl _strlwr_l(char *, _locale_t);
extern "C" char *__cdecl _strupr_l(char *, _locale_t);

TEST_CASE("_strlwr _strupr")
{
  char buf[16];

  strcpy(buf, "AbC");
  REQUIRE(_strlwr(buf) == buf);
  REQUIRE(strcmp(buf, "abc") == 0);

  strcpy(buf, "aBc");
  REQUIRE(_strupr(buf) == buf);
  REQUIRE(strcmp(buf, "ABC") == 0);

  // UTF-8 sequence bytes pass through untouched (wine's native garbles
  // them via its ACP round trip — the M8 anchoring probe caught
  // e4 bda0 -> e4 bdab + truncation; ours is byte-stable)
  const unsigned char ni[] = {0xe4, 0xbd, 0xa0}; // 你
  memcpy(buf, ni, 3);
  buf[3] = 'A';
  buf[4] = 'b';
  buf[5] = 0;
  REQUIRE(_strlwr(buf) == buf);
  REQUIRE(memcmp(buf, ni, 3) == 0);
  REQUIRE(strcmp(buf + 3, "ab") == 0);

  memcpy(buf, ni, 3);
  buf[3] = 'c';
  buf[4] = 'D';
  buf[5] = 0;
  REQUIRE(_strupr(buf) == buf);
  REQUIRE(memcmp(buf, ni, 3) == 0);
  REQUIRE(strcmp(buf + 3, "CD") == 0);

  // wine anchor: null -> NULL + EINVAL
  errno = 0;
  REQUIRE(_strlwr(nullptr) == nullptr);
  REQUIRE(errno == EINVAL);
  errno = 0;
  REQUIRE(_strupr(nullptr) == nullptr);
  REQUIRE(errno == EINVAL);

  // _l delegates
  strcpy(buf, "AbC");
  REQUIRE(_strlwr_l(buf, nullptr) == buf);
  REQUIRE(strcmp(buf, "abc") == 0);
  strcpy(buf, "aBc");
  REQUIRE(_strupr_l(buf, nullptr) == buf);
  REQUIRE(strcmp(buf, "ABC") == 0);
}

TEST_CASE("_strlwr_s _strupr_s")
{
  char buf[16];

  // success
  strcpy(buf, "AbC");
  errno = 0;
  REQUIRE(_strlwr_s(buf, sizeof buf) == 0);
  REQUIRE(strcmp(buf, "abc") == 0);
  REQUIRE(errno == 0);

  strcpy(buf, "aBc");
  REQUIRE(_strupr_s(buf, sizeof buf) == 0);
  REQUIRE(strcmp(buf, "ABC") == 0);

  // wine anchors: size 0 -> EINVAL with the buffer UNTOUCHED
  strcpy(buf, "AbC");
  errno = 0;
  REQUIRE(_strlwr_s(buf, 0) == EINVAL);
  REQUIRE(errno == EINVAL);
  REQUIRE(buf[0] == 'A');

  strcpy(buf, "AbC");
  REQUIRE(_strupr_s(buf, 0) == EINVAL);
  REQUIRE(buf[0] == 'A');

  // too small / not terminated within size -> EINVAL + str[0] reset
  strcpy(buf, "AbC");
  errno = 0;
  REQUIRE(_strlwr_s(buf, 2) == EINVAL);
  REQUIRE(errno == EINVAL);
  REQUIRE(buf[0] == 0);

  strcpy(buf, "aBc");
  REQUIRE(_strupr_s(buf, 2) == EINVAL);
  REQUIRE(buf[0] == 0);

  // null -> EINVAL
  errno = 0;
  REQUIRE(_strlwr_s(nullptr, 8) == EINVAL);
  REQUIRE(errno == EINVAL);
  REQUIRE(_strupr_s(nullptr, 8) == EINVAL);

  // exact fit (strlen + terminator) succeeds
  strcpy(buf, "Ab");
  REQUIRE(_strlwr_s(buf, 3) == 0);
  REQUIRE(strcmp(buf, "ab") == 0);

  // CJK bytes are case-less and must survive intact
  const unsigned char ni[] = {0xe4, 0xbd, 0xa0};
  memcpy(buf, ni, 3);
  buf[3] = 'B';
  buf[4] = 0;
  REQUIRE(_strlwr_s(buf, sizeof buf) == 0);
  REQUIRE(memcmp(buf, ni, 3) == 0);
  REQUIRE(buf[3] == 'b');
}
