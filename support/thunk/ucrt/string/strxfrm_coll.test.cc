#include <catch_amalgamated.hpp>

#include <string.h>

extern "C" size_t __cdecl strxfrm(char *, const char *, size_t);
#include <locale.h>

extern "C" size_t __cdecl _strxfrm_l(char *, const char *, size_t, _locale_t);
extern "C" int __cdecl _stricoll(const char *, const char *);
extern "C" int __cdecl _stricoll_l(const char *, const char *, _locale_t);
extern "C" int __cdecl _strncoll(const char *, const char *, size_t);
extern "C" int __cdecl _strnicoll(const char *, const char *, size_t);
extern "C" int __cdecl _strnicoll_l(const char *, const char *, size_t, _locale_t);

TEST_CASE("strxfrm identity")
{
  char dst[16];

  memset(dst, 0, sizeof dst);
  REQUIRE(strxfrm(dst, "abc", sizeof dst) == 3);
  REQUIRE(strcmp(dst, "abc") == 0);

  // wine anchor: n == 0 (dst may be null) -> required length only
  REQUIRE(strxfrm(nullptr, "abc", 0) == 3);
  REQUIRE(strxfrm(dst, "abc", 0) == 3);
  REQUIRE(strcmp(dst, "abc") == 0); // untouched

  // truncation: copies exactly count bytes, returns full length
  memset(dst, 0, sizeof dst);
  REQUIRE(strxfrm(dst, "abcdef", 3) == 6);
  REQUIRE(dst[0] == 'a');
  REQUIRE(dst[1] == 'b');
  REQUIRE(dst[2] == 'c');

  // padding: a source shorter than the window is NUL-filled to count,
  // which is the reference's strncpy and wine's observable answer.  The
  // bytes past count are not touched.
  memset(dst, 'x', sizeof dst);
  REQUIRE(strxfrm(dst, "abc", 6) == 3);
  REQUIRE(memcmp(dst, "abc\0\0\0", 6) == 0);
  REQUIRE(dst[6] == 'x');

  REQUIRE(strxfrm(dst, "", sizeof dst) == 0);

  // UTF-8 bytes are carried verbatim (code point order == byte order)
  const char ni[] = "\xe4\xbd\xa0\xe5\xa5\xbd"; // 你好
  memset(dst, 0, sizeof dst);
  REQUIRE(strxfrm(dst, ni, sizeof dst) == strlen(ni));
  REQUIRE(strcmp(dst, ni) == 0);

  // _l variant delegates identically
  memset(dst, 0, sizeof dst);
  REQUIRE(_strxfrm_l(dst, "abc", sizeof dst, nullptr) == 3);
  REQUIRE(strcmp(dst, "abc") == 0);
}

TEST_CASE("coll family")
{
  // fold-difference shapes (wine anchors)
  REQUIRE(_stricoll("A", "b") == -1);
  REQUIRE(_stricoll("B", "a") == 1);
  REQUIRE(_stricoll("abc", "ABC") == 0);
  REQUIRE(_stricoll("", "") == 0);
  REQUIRE(_stricoll("\xe4\xbd\xa0", "\xe4\xbd\xa0") == 0);
  REQUIRE(_stricoll_l("A", "b", nullptr) == -1);
  REQUIRE(_stricoll_l("abc", "ABC", nullptr) == 0);

  // byte-order (strncmp mirror; wine anchor: equal prefix -> 0)
  REQUIRE(_strncoll("abcd", "abef", 2) == 0);
  REQUIRE(_strncoll("abc", "abd", 3) == 'c' - 'd');
  REQUIRE(_strncoll("abc", "abc", 3) == 0);
  REQUIRE(_strncoll_l("abc", "abd", 3, nullptr) == 'c' - 'd');

  // fold-difference, count limited
  REQUIRE(_strnicoll("AB", "ab", 2) == 0);
  REQUIRE(_strnicoll("ABc", "abd", 2) == 0);
  REQUIRE(_strnicoll("ABc", "abd", 3) == 'c' - 'd');
  REQUIRE(_strnicoll_l("AB", "ab", 2, nullptr) == 0);

  // code point order: 你 (U+4F60) sorts after all ASCII
  REQUIRE(_strncoll("\xe4\xbd\xa0", "z", 4) > 0);
  REQUIRE(_stricoll("\xe4\xbd\xa0", "z") > 0);
}
