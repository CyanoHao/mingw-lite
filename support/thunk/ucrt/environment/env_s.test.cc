#include <catch_amalgamated.hpp>

#include <direct.h>
#include <errno.h>
#include <io.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <wchar.h>

// all path/env faces used here are declared by the mingw headers; the
// overlay definitions win at link time

// native pollution vector (def-backed alias, environment def)
extern "C" __attribute__((dllimport)) int __cdecl
__ms_putenv(const char *string);

static const char U8_HAO[] = "\xe4\xbd\xa0\xe5\xa5\xbd"; // UTF-8 "你好"

TEST_CASE("env _s CJK round chain")
{
  // wide set -> UTF-8 narrow view -> counted copy -> duplicate -> delete
  REQUIRE(_wputenv_s(L"U8CJT", L"\x4f60\x597d") == 0);

  const char *value = getenv("U8CJT");
  REQUIRE(value != nullptr);
  REQUIRE(strcmp(value, U8_HAO) == 0);

  size_t n = 99;
  char buf[32];
  errno = 0;
  REQUIRE(getenv_s(&n, buf, sizeof buf, "U8CJT") == 0);
  REQUIRE(n == 7); // bytes including NUL
  REQUIRE(strcmp(buf, U8_HAO) == 0);
  REQUIRE(errno == 0);

  // exact capacity succeeds, one short returns ERANGE with the size
  n = 99;
  REQUIRE(getenv_s(&n, buf, 7, "U8CJT") == 0);
  buf[0] = 'X';
  n = 99;
  errno = 0;
  REQUIRE(getenv_s(&n, buf, 6, "U8CJT") == ERANGE);
  REQUIRE(n == 7);
  REQUIRE(buf[0] == 0); // cleared up front, never written into
  REQUIRE(errno == 0);  // ERANGE does not set errno (wine anchor)

  // query mode
  n = 99;
  REQUIRE(getenv_s(&n, nullptr, 0, "U8CJT") == 0);
  REQUIRE(n == 7);

  // duplicate is a plain malloc block the caller frees
  char *dup = reinterpret_cast<char *>(0x1);
  n = 99;
  REQUIRE(_dupenv_s(&dup, &n, "U8CJT") == 0);
  REQUIRE(dup != nullptr);
  REQUIRE(n == 7);
  REQUIRE(strcmp(dup, U8_HAO) == 0);
  free(dup);

  // empty value deletes the variable (wine anchor)
  REQUIRE(_putenv_s("U8CJT", "") == 0);
  REQUIRE(getenv("U8CJT") == nullptr);
}

TEST_CASE("getenv_s protocol matrix")
{
  _putenv_s("U8PA", "hello");

  // miss is not an error: rc=0, len=0, buffer cleared
  size_t n = 99;
  char buf[16];
  buf[0] = 'X';
  REQUIRE(getenv_s(&n, buf, sizeof buf, "U8MISSING") == 0);
  REQUIRE(n == 0);
  REQUIRE(buf[0] == 0);

  // null name behaves as a miss (r4)
  n = 99;
  buf[0] = 'X';
  REQUIRE(getenv_s(&n, buf, sizeof buf, nullptr) == 0);
  REQUIRE(n == 0);
  REQUIRE(buf[0] == 0);

  // null count slot -> EINVAL (r1)
  errno = 0;
  REQUIRE(getenv_s(nullptr, buf, sizeof buf, "U8PA") == EINVAL);
  REQUIRE(errno == EINVAL);

  // buffer/count pairing -> EINVAL, count slot already zeroed (r2/r3)
  n = 99;
  errno = 0;
  REQUIRE(getenv_s(&n, nullptr, 5, "U8PA") == EINVAL);
  REQUIRE(n == 0);
  n = 99;
  buf[0] = 'X';
  errno = 0;
  REQUIRE(getenv_s(&n, buf, 0, "U8PA") == EINVAL);
  REQUIRE(n == 0);
  REQUIRE(buf[0] == 'X'); // untouched
}

TEST_CASE("_dupenv_s protocol matrix")
{
  _putenv_s("U8PA", "hello");

  // miss: rc=0, ptr=NULL, count=0
  char *dup = reinterpret_cast<char *>(0x1);
  size_t n = 99;
  REQUIRE(_dupenv_s(&dup, &n, "U8MISSING") == 0);
  REQUIRE(dup == nullptr);
  REQUIRE(n == 0);

  // null slot -> EINVAL, nothing written (r5)
  n = 99;
  errno = 0;
  REQUIRE(_dupenv_s(nullptr, &n, "U8PA") == EINVAL);
  REQUIRE(errno == EINVAL);
  REQUIRE(n == 99);

  // null name -> EINVAL, both slots untouched (r6)
  dup = reinterpret_cast<char *>(0x1);
  n = 99;
  errno = 0;
  REQUIRE(_dupenv_s(&dup, &n, nullptr) == EINVAL);
  REQUIRE(errno == EINVAL);
  // The sentinel is 0x1, not a pointer to a real byte, so the comparison
  // is on the address as an integer: Catch2's reporter stringifies a
  // `char *` expansion by reading it, and running the suite with `-s`
  // (show successful assertions) has it read address 0x1 and fault.
  REQUIRE(reinterpret_cast<uintptr_t>(dup) == 1);
  REQUIRE(n == 99);

  // count slot optional
  dup = nullptr;
  REQUIRE(_dupenv_s(&dup, nullptr, "U8PA") == 0);
  REQUIRE(strcmp(dup, "hello") == 0);
  free(dup);
}

TEST_CASE("_putenv_s validation")
{
  errno = 0;
  REQUIRE(_putenv_s(nullptr, "v") == EINVAL);
  REQUIRE(errno == EINVAL);
  REQUIRE(_putenv_s("U8PA", nullptr) == EINVAL);
  REQUIRE(_putenv_s("", "v") == EINVAL);
  REQUIRE(_putenv_s("K=X", "v") == EINVAL);
  REQUIRE(_wputenv_s(L"K=X", L"v") == EINVAL);
  REQUIRE(_wputenv_s(L"U8PA", nullptr) == EINVAL);

  // plain set + create + verify through the UTF-8 environment
  REQUIRE(_putenv_s("U8NEW", U8_HAO) == 0);
  const char *value = getenv("U8NEW");
  REQUIRE(value != nullptr);
  REQUIRE(strcmp(value, U8_HAO) == 0);
}

TEST_CASE("_searchenv walk")
{
  FILE *f = fopen("u8env_probe.txt", "wb");
  REQUIRE(f != nullptr);
  fputs("x", f);
  fclose(f);

  _putenv_s("U8DIR", ".");
  _mkdir("u8env_sub");
  f = fopen("u8env_sub/in.txt", "wb");
  REQUIRE(f != nullptr);
  fputs("x", f);
  fclose(f);
  _putenv_s("U8DIR2", "u8env_sub;.");

  char buf[_MAX_PATH];

  // cwd hit -> fully qualified (ends with the file name)
  buf[0] = 'X';
  errno = 0;
  _searchenv("u8env_probe.txt", "PATH", buf);
  REQUIRE(buf[0] != 0);
  size_t len = strlen(buf);
  REQUIRE(strcmp(buf + len - strlen("u8env_probe.txt"), "u8env_probe.txt") ==
          0);
  REQUIRE(buf[len - strlen("u8env_probe.txt") - 1] == '\\');

  // component hit through "." -> full path again
  _searchenv("u8env_probe.txt", "U8DIR", buf);
  REQUIRE(strstr(buf, "u8env_probe.txt") != nullptr);

  // subdir component hit -> relative concatenation shape (probe D)
  buf[0] = 'X';
  errno = 0;
  _searchenv("in.txt", "U8DIR2", buf);
  REQUIRE(strcmp(buf, "u8env_sub\\in.txt") == 0);
  REQUIRE(errno == 0);

  // miss -> empty buffer + ENOENT
  buf[0] = 'X';
  errno = 0;
  _searchenv("no_such_u8_zzz.bin", "U8DIR", buf);
  REQUIRE(buf[0] == 0);
  REQUIRE(errno == ENOENT);

  // missing env var -> miss (the direct cwd probe runs first, so the
  // file must NOT exist — reference order, probe D used a missing file)
  buf[0] = 'X';
  errno = 0;
  _searchenv("no_such_u8_zzz.bin", "U8NOVAR", buf);
  REQUIRE(buf[0] == 0);
  REQUIRE(errno == ENOENT);

  // empty file name: reference semantics (buf cleared + ENOENT);
  // wine leaves garbage here (R-1, wine shell-bug candidate)
  buf[0] = 'X';
  errno = 0;
  _searchenv("", "PATH", buf);
  REQUIRE(buf[0] == 0);
  REQUIRE(errno == ENOENT);

  // bounded engine: tiny -> ERANGE + cleared; miss -> ENOENT
  errno_t rc;
  buf[0] = 'X';
  errno = 0;
  rc = _searchenv_s("u8env_probe.txt", "U8DIR", buf, 4);
  REQUIRE(rc == ERANGE);
  REQUIRE(buf[0] == 0);
  REQUIRE(errno == ERANGE);

  buf[0] = 'X';
  errno = 0;
  rc = _searchenv_s("no_such_u8_zzz.bin", "U8DIR", buf, sizeof buf);
  REQUIRE(rc == ENOENT);
  REQUIRE(buf[0] == 0);
  REQUIRE(errno == ENOENT);

  // r14: null file / null buffer / zero count -> EINVAL, untouched
  buf[0] = 'X';
  errno = 0;
  rc = _searchenv_s(nullptr, "PATH", buf, sizeof buf);
  REQUIRE(rc == EINVAL);
  REQUIRE(buf[0] == 'X');
  REQUIRE(_searchenv_s("u8env_probe.txt", "PATH", nullptr, 64) == EINVAL);
  buf[0] = 'X';
  REQUIRE(_searchenv_s("u8env_probe.txt", "PATH", buf, 0) == EINVAL);
  REQUIRE(buf[0] == 'X');

  remove("u8env_probe.txt");
  remove("u8env_sub/in.txt");
  _rmdir("u8env_sub");
}

TEST_CASE("env reads never touch the native environment")
{
  // drive the native environment behind our back — the overlay reads
  // its own UTF-8 musl environ only
  REQUIRE(__ms_putenv("U8NATIVE=native_value") == 0);
  REQUIRE(getenv("U8NATIVE") == nullptr);

  size_t n = 99;
  char buf[32];
  REQUIRE(getenv_s(&n, buf, sizeof buf, "U8NATIVE") == 0);
  REQUIRE(n == 0);

  char *dup = reinterpret_cast<char *>(0x1);
  n = 99;
  REQUIRE(_dupenv_s(&dup, &n, "U8NATIVE") == 0);
  REQUIRE(dup == nullptr);
  REQUIRE(n == 0);
}
