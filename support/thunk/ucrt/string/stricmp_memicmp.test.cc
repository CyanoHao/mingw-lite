#include <catch_amalgamated.hpp>

#include <ctype.h>
#include <locale.h>
#include <string.h>
#include <wchar.h>

extern "C" int __cdecl _stricmp_l(const char *, const char *, _locale_t);
extern "C" int __cdecl _strnicmp_l(const char *, const char *, size_t,
                                   _locale_t);
extern "C" int __cdecl _memicmp_l(const void *, const void *, size_t,
                                  _locale_t);

TEST_CASE("_stricmp")
{
  // wine anchors: folded byte difference, not a normalized sign
  REQUIRE(_stricmp("a", "c") == -2);
  REQUIRE(_stricmp("c", "a") == 2);
  REQUIRE(_stricmp("A", "a") == 0);
  REQUIRE(_stricmp("b", "d") == -2);
  REQUIRE(_stricmp("hello", "HELLO") == 0);
  REQUIRE(_stricmp("", "") == 0);
  REQUIRE(_stricmp("abc", "") == 'a');

  // UTF-8 bytes compare raw (no byte-level fold for >= 0x80)
  REQUIRE(_stricmp("\xe4\xbd\xa0", "\xe4\xbd\xa0") == 0);   // 你
  REQUIRE(_stricmp("\xe4\xbd\xa0", "\xe4\xbd\xa1") == -1);  // last byte differs

  // locale argument ignored
  REQUIRE(_stricmp_l("Ab", "aB", nullptr) == 0);
  REQUIRE(_stricmp_l("a", "c", nullptr) == -2);
}

TEST_CASE("_strnicmp")
{
  REQUIRE(_strnicmp("Ab", "aC", 2) == -1); // 'b' - 'c' (wine anchor)
  REQUIRE(_strnicmp("AB", "ab", 2) == 0);
  REQUIRE(_strnicmp("ABc", "ab", 2) == 0);
  REQUIRE(_strnicmp("abc", "abd", 2) == 0);
  REQUIRE(_strnicmp("abc", "abd", 3) == -1);
  REQUIRE(_strnicmp("A", "a", 0) == 0);
  REQUIRE(_strnicmp("a", "", 1) == 'a');
  REQUIRE(_strnicmp_l("Ab", "aC", 2, nullptr) == -1);
}

TEST_CASE("_memicmp")
{
  REQUIRE(_memicmp("AB", "ac", 2) == -1); // 'b' - 'c' (wine anchor)
  REQUIRE(_memicmp("ab", "ac", 2) == -1);
  REQUIRE(_memicmp("ab", "ab", 2) == 0);
  REQUIRE(_memicmp("ab", "ab", 0) == 0);
  REQUIRE(_memicmp("abX", "abY", 2) == 0); // difference beyond the count
  REQUIRE(_memicmp("ab", "aa", 2) == 'b' - 'a');
  REQUIRE(_memicmp("\xe4\xbd", "\xe4\xbd", 2) == 0); // UTF-8 bytes raw
  REQUIRE(_memicmp_l("AB", "ac", 2, nullptr) == -1);
}
