#include <catch_amalgamated.hpp>

#include <direct.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

// IAT-level tests for _getdcwd (plan §M5.3-2): the wide side stays
// native (_wgetdcwd plain dllimport — no __ms_ face), the narrow
// answer is transcoded from the wide path.  _getdcwd is declared by
// direct.h.  (A bare getdcwd forwarder shipped here from M5 to the
// 2026-09-29 audit; it is gone now — no def, no header, no import
// library ever offered that spelling, so nothing could reach it.)

TEST_CASE("_getdcwd")
{
  char a[512], b[512];

  // drive 0 (current drive) agrees with _getcwd byte for byte
  REQUIRE(_getcwd(a, sizeof a) == a);
  REQUIRE(_getdcwd(0, b, sizeof b) == b);
  REQUIRE(strcmp(a, b) == 0);

  // small buffer: NULL + ERANGE (every cwd is at least "X:\\" = 3)
  errno = 0;
  REQUIRE(_getdcwd(0, b, 2) == nullptr);
  REQUIRE(errno == ERANGE);

  // null buffer: malloc shape, caller frees
  char *p = _getdcwd(0, nullptr, 0);
  REQUIRE(p != nullptr);
  REQUIRE(strlen(p) > 0);
  free(p);
}

TEST_CASE("_getdcwd UTF-8 cwd round trip")
{
  // build a Chinese-named directory and enter it via Win32 (UTF-16
  // correct — the CRT _chdir/_mkdir natives walk the ACP), then read
  // the cwd back through the narrow thunk
  wchar_t w_orig[MAX_PATH];
  REQUIRE(GetCurrentDirectoryW(MAX_PATH, w_orig) > 0);
  REQUIRE(CreateDirectoryW(L"test-cwd-你好", nullptr) != 0);
  REQUIRE(SetCurrentDirectoryW(L"test-cwd-你好") != 0);

  char buf[MAX_PATH];
  REQUIRE(_getdcwd(0, buf, sizeof buf) == buf);

  // the path ends in "test-cwd-你好" (9 ASCII + 3 + 3 UTF-8 bytes)
  size_t len = strlen(buf);
  REQUIRE(len >= 15);
  REQUIRE(strcmp(buf + len - 15,
                 "test-cwd-"
                 "\xe4\xbd\xa0"
                 "\xe5\xa5\xbd") == 0);

  // restore and clean up
  REQUIRE(SetCurrentDirectoryW(w_orig) != 0);
  REQUIRE(RemoveDirectoryW(L"test-cwd-你好") != 0);
}
