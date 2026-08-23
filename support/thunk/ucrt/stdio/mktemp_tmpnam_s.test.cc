#include <catch_amalgamated.hpp>

#include <errno.h>
#include <io.h>
#include <stdio.h>
#include <string.h>

extern "C" int __cdecl _mktemp_s(char *template_string, size_t size);
extern "C" int __cdecl tmpnam_s(char *buffer, size_t size);

TEST_CASE("mktemp")
{
  char tpl[64];
  const char prefix[] = "\xe6\xb5\x8b\xe8\xaf\x95m7-"; // 测试m7-

  strcpy(tpl, prefix);
  strcat(tpl, "XXXXXX");
  errno = 0;
  char *result = _mktemp(tpl);
  REQUIRE(result == tpl);
  REQUIRE(errno == 0); // reference restores errno (wine leaks ENOENT)

  size_t plen = strlen(prefix);
  size_t len = strlen(tpl);
  REQUIRE(len == plen + 6);          // 1 letter + 5 thread-id digits
  REQUIRE(tpl[plen] >= 'a');
  REQUIRE(tpl[plen] <= 'z');
  for (size_t i = plen + 1; i < len; i++) {
    REQUIRE(tpl[i] >= '0');
    REQUIRE(tpl[i] <= '9');
  }
  // mktemp only names, never creates
  REQUIRE(_access(tpl, 0) == -1);

  // no X-run of six: nullptr + EINVAL (reference shape; wine sets no
  // errno here — noted divergence)
  strcpy(tpl, prefix);
  strcat(tpl, "XXXXX");
  errno = 0;
  REQUIRE(_mktemp(tpl) == nullptr);
  REQUIRE(errno == EINVAL);

  strcpy(tpl, "abc");
  errno = 0;
  REQUIRE(_mktemp(tpl) == nullptr);
  REQUIRE(errno == EINVAL);
}

TEST_CASE("mktemp_s")
{
  char tpl[64];

  strcpy(tpl, "m7probe-XXXXXX");
  REQUIRE(_mktemp_s(tpl, sizeof tpl) == 0);
  REQUIRE(strlen(tpl) == 8 + 6);
  REQUIRE(tpl[8] >= 'a');
  REQUIRE(tpl[8] <= 'z');
  REQUIRE(_access(tpl, 0) == -1);

  // wine anchors: null / zero size -> EINVAL with template untouched
  errno = 0;
  REQUIRE(_mktemp_s(nullptr, 8) == EINVAL);
  REQUIRE(errno == EINVAL);
  strcpy(tpl, "m7probe-XXXXXX");
  errno = 0;
  REQUIRE(_mktemp_s(tpl, 0) == EINVAL);
  REQUIRE(errno == EINVAL);
  REQUIRE(tpl[0] == 'm');

  // size too small to hold the template: EINVAL + reset
  strcpy(tpl, "m7probe-XXXXXX");
  REQUIRE(_mktemp_s(tpl, 3) == EINVAL);
  REQUIRE(tpl[0] == 0);

  // no X-run: EINVAL + reset
  strcpy(tpl, "abc");
  REQUIRE(_mktemp_s(tpl, 4) == EINVAL);
  REQUIRE(tpl[0] == 0);
}

TEST_CASE("mktemp_s exhaustion")
{
  char tpl[64];
  strcpy(tpl, "m7exh-XXXXXX");
  REQUIRE(_mktemp_s(tpl, sizeof tpl) == 0);

  // occupy every probe letter of this thread-id suffix
  size_t len = strlen(tpl);
  char names[27][64];
  for (int i = 0; i < 26; i++) {
    strcpy(names[i], tpl);
    names[i][len - 6] = char('a' + i);
    FILE *fp = fopen(names[i], "wb");
    REQUIRE(fp);
    REQUIRE(fclose(fp) == 0);
  }

  strcpy(tpl, "m7exh-XXXXXX");
  errno = 0;
  REQUIRE(_mktemp_s(tpl, sizeof tpl) == EEXIST);
  REQUIRE(errno == EEXIST);
  REQUIRE(tpl[0] == 0);

  for (int i = 0; i < 26; i++)
    REQUIRE(remove(names[i]) == 0);
}

TEST_CASE("tmpnam_s")
{
  char buf[L_tmpnam];

  errno = 0;
  REQUIRE(tmpnam_s(buf, L_tmpnam) == 0);
  REQUIRE(buf[0] != 0);
  REQUIRE(strlen(buf) > 0);

  // wine anchors: tiny buffer -> ERANGE with buf[0] reset; zero size
  // -> ERANGE; null buffer -> EINVAL
  errno = 0;
  REQUIRE(tmpnam_s(buf, 2) == ERANGE);
  REQUIRE(errno == ERANGE);
  REQUIRE(buf[0] == 0);

  errno = 0;
  REQUIRE(tmpnam_s(buf, 0) == ERANGE);
  REQUIRE(errno == ERANGE);

  errno = 0;
  REQUIRE(tmpnam_s(nullptr, L_tmpnam) == EINVAL);
  REQUIRE(errno == EINVAL);
}
