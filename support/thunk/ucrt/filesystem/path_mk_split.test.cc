#include <catch_amalgamated.hpp>

#include <errno.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// all path faces used here are declared by the mingw headers; the
// overlay definitions win at link time

// native pollution vector (def-backed alias, locale def)
extern "C" __attribute__((dllimport)) int __cdecl
__ms__setmbcp(int code_page);

TEST_CASE("makepath compose")
{
  char buf[260];

  _makepath(buf, "C", "\\dir\\", "file", ".txt");
  REQUIRE(strcmp(buf, "C:\\dir\\file.txt") == 0);

  // drive contributes its first character + ':' (probe E)
  _makepath(buf, "CD", "d", "f", "t");
  REQUIRE(strcmp(buf, "C:d\\f.t") == 0);

  _makepath(buf, nullptr, nullptr, "file", nullptr);
  REQUIRE(strcmp(buf, "file") == 0);

  // extension without the dot gets one; empty extension does not
  _makepath(buf, nullptr, nullptr, "file", "txt");
  REQUIRE(strcmp(buf, "file.txt") == 0);
  _makepath(buf, "C", "d", "f", "");
  REQUIRE(strcmp(buf, "C:d\\f") == 0);

  // directory without a trailing separator gets one
  _makepath(buf, nullptr, "d/e", "f", nullptr);
  REQUIRE(strcmp(buf, "d/e\\f") == 0);

  // CJK bytes pass through untouched
  _makepath(buf, "C", "\xe4\xb8\xad\xe6\x96\x87\\", "f", "txt");
  REQUIRE(strcmp(buf, "C:\xe4\xb8\xad\xe6\x96\x87\\f.txt") == 0);

  // 0xC2 before a separator: the separator is real (continuation
  // bytes are >= 0x80), the trailing-'\\' check must NOT insert one
  // after 0xC2 — and no DBCS lead swallowing exists here
  _makepath(buf, nullptr, "dir\xc2\\", "f", "txt");
  REQUIRE(strcmp(buf, "dir\xc2\\f.txt") == 0);
}

TEST_CASE("makepath_s capacity")
{
  char buf[16];

  errno = 0;
  REQUIRE(_makepath_s(buf, sizeof buf, "C", "\\dir\\", "file", ".txt") == 0);
  REQUIRE(strcmp(buf, "C:\\dir\\file.txt") == 0);
  REQUIRE(errno == 0);

  // exact capacity (strlen + 1 = 11) succeeds; anything less ERANGEs
  // ("C:\d\f.txt" is 10 characters — the probe comment miscounted)
  errno = 0;
  REQUIRE(_makepath_s(buf, 11, "C", "\\d\\", "f", ".txt") == 0);
  REQUIRE(strcmp(buf, "C:\\d\\f.txt") == 0);

  // no room for the NUL -> ERANGE + cleared + errno (probe E)
  buf[0] = 'X';
  errno = 0;
  REQUIRE(_makepath_s(buf, 10, "C", "\\d\\", "f", ".txt") == ERANGE);
  REQUIRE(buf[0] == 0);
  REQUIRE(errno == ERANGE);
  buf[0] = 'X';
  errno = 0;
  REQUIRE(_makepath_s(buf, 9, "C", "\\d\\", "f", ".txt") == ERANGE);
  REQUIRE(buf[0] == 0);

  // null buffer / zero count -> EINVAL, nothing written (r11)
  errno = 0;
  REQUIRE(_makepath_s(nullptr, 0, "C", "d", "f", "t") == EINVAL);
  REQUIRE(errno == EINVAL);
  buf[0] = 'X';
  errno = 0;
  REQUIRE(_makepath_s(buf, 0, "C", "d", "f", "t") == EINVAL);
  REQUIRE(buf[0] == 'X');
}

TEST_CASE("splitpath decompose")
{
  char drive[8], dir[64], fname[64], ext[16];
  errno_t rc;

  rc = _splitpath_s("C:\\dir\\sub\\file.txt", drive, sizeof drive, dir,
                    sizeof dir, fname, sizeof fname, ext, sizeof ext);
  REQUIRE(rc == 0);
  REQUIRE(strcmp(drive, "C:") == 0);
  REQUIRE(strcmp(dir, "\\dir\\sub\\") == 0);
  REQUIRE(strcmp(fname, "file") == 0);
  REQUIRE(strcmp(ext, ".txt") == 0);

  // no drive: slot cleared
  rc = _splitpath_s("dir\\file", drive, sizeof drive, dir, sizeof dir,
                    fname, sizeof fname, ext, sizeof ext);
  REQUIRE(rc == 0);
  REQUIRE(drive[0] == 0);
  REQUIRE(strcmp(dir, "dir\\") == 0);
  REQUIRE(strcmp(fname, "file") == 0);
  REQUIRE(ext[0] == 0);

  // dot in the directory does not split the name
  rc = _splitpath_s("a.b\\file", drive, sizeof drive, dir, sizeof dir,
                    fname, sizeof fname, ext, sizeof ext);
  REQUIRE(rc == 0);
  REQUIRE(strcmp(dir, "a.b\\") == 0);
  REQUIRE(strcmp(fname, "file") == 0);
  REQUIRE(ext[0] == 0);

  // hidden file and trailing dot shapes (probe E)
  rc = _splitpath_s(".bashrc", drive, sizeof drive, dir, sizeof dir, fname,
                    sizeof fname, ext, sizeof ext);
  REQUIRE(rc == 0);
  REQUIRE(fname[0] == 0);
  REQUIRE(strcmp(ext, ".bashrc") == 0);

  rc = _splitpath_s("file.", drive, sizeof drive, dir, sizeof dir, fname,
                    sizeof fname, ext, sizeof ext);
  REQUIRE(rc == 0);
  REQUIRE(strcmp(fname, "file") == 0);
  REQUIRE(strcmp(ext, ".") == 0);

  // unbounded variant with null slots
  _splitpath("C:\\d\\f.txt", drive, dir, nullptr, nullptr);
  REQUIRE(strcmp(drive, "C:") == 0);
  REQUIRE(strcmp(dir, "\\d\\") == 0);
}

TEST_CASE("splitpath failure shapes")
{
  char drive[8], dir[64], fname[64], ext[16];
  errno_t rc;

  // tiny ext slot -> ERANGE with ALL non-null slots cleared (probe E)
  char tiny[1];
  tiny[0] = 'X';
  errno = 0;
  rc = _splitpath_s("C:\\d\\f.txt", drive, sizeof drive, dir, sizeof dir,
                    fname, sizeof fname, tiny, sizeof tiny);
  REQUIRE(rc == ERANGE);
  REQUIRE(drive[0] == 0);
  REQUIRE(dir[0] == 0);
  REQUIRE(fname[0] == 0);
  REQUIRE(tiny[0] == 0);
  REQUIRE(errno == ERANGE);

  // null path -> EINVAL, every buffer untouched (r12)
  memset(drive, 'X', sizeof drive);
  memset(dir, 'X', sizeof dir);
  memset(fname, 'X', sizeof fname);
  memset(ext, 'X', sizeof ext);
  errno = 0;
  rc = _splitpath_s(nullptr, drive, sizeof drive, dir, sizeof dir, fname,
                    sizeof fname, ext, sizeof ext);
  REQUIRE(rc == EINVAL);
  REQUIRE(drive[0] == 'X');
  REQUIRE(dir[0] == 'X');
  REQUIRE(fname[0] == 'X');
  REQUIRE(ext[0] == 'X');
  REQUIRE(errno == EINVAL);

  // slot/count mismatch -> EINVAL, untouched (r13)
  memset(dir, 'X', sizeof dir);
  errno = 0;
  rc = _splitpath_s("C:\\d\\f.txt", drive, 0, dir, sizeof dir, fname,
                    sizeof fname, ext, sizeof ext);
  REQUIRE(rc == EINVAL);
  REQUIRE(dir[0] == 'X');
  REQUIRE(errno == EINVAL);
}

TEST_CASE("splitpath utf-8 self-synchronization")
{
  // R1 reversal: under native CP936 the 0xC2 swallows the backslash
  // (probe F: dir empty, file "dir\xc2\\f"); the UTF-8 byte walk keeps
  // them apart because 0x5C can never be a continuation byte
  char drive[8], dir[64], fname[64], ext[16];
  errno_t rc = _splitpath_s("dir\xc2\\f.txt", drive, sizeof drive, dir,
                            sizeof dir, fname, sizeof fname, ext,
                            sizeof ext);
  REQUIRE(rc == 0);
  REQUIRE(strcmp(dir, "dir\xc2\\") == 0);
  REQUIRE(strcmp(fname, "f") == 0);
  REQUIRE(strcmp(ext, ".txt") == 0);

  // truncated sequence tail: the lone lead byte is just a character
  rc = _splitpath_s("dir\xc2", drive, sizeof drive, dir, sizeof dir, fname,
                    sizeof fname, ext, sizeof ext);
  REQUIRE(rc == 0);
  REQUIRE(dir[0] == 0);
  REQUIRE(strcmp(fname, "dir\xc2") == 0);
  REQUIRE(ext[0] == 0);

  // with the native DBCS state driven to CP936 behind our back the
  // decomposition must not drift (our _setmbcp is a no-op; native
  // state is unreachable from the overlay)
  REQUIRE(__ms__setmbcp(936) == 0);
  rc = _splitpath_s("dir\xc2\\f.txt", drive, sizeof drive, dir, sizeof dir,
                    fname, sizeof fname, ext, sizeof ext);
  REQUIRE(rc == 0);
  REQUIRE(strcmp(dir, "dir\xc2\\") == 0);
  REQUIRE(strcmp(fname, "f") == 0);
  __ms__setmbcp(65001);

  // round trip: split then make rebuilds the path
  const char *original = "C:\\dir\xe4\xb8\xad\\file.txt";
  rc = _splitpath_s(original, drive, sizeof drive, dir, sizeof dir, fname,
                    sizeof fname, ext, sizeof ext);
  REQUIRE(rc == 0);
  char rebuilt[260];
  _makepath(rebuilt, drive, dir, fname, ext);
  REQUIRE(strcmp(rebuilt, original) == 0);
}

TEST_CASE("_access_s matrix")
{
  FILE *f = fopen("u8acc_probe.txt", "wb");
  REQUIRE(f != nullptr);
  fputs("x", f);
  fclose(f);

  errno = 0;
  REQUIRE(_access_s("u8acc_probe.txt", 0) == 0);
  REQUIRE(_access_s("u8acc_probe.txt", 4) == 0);
  REQUIRE(_access_s("u8acc_probe.txt", 6) == 0);

  // missing -> ENOENT + errno (probe D)
  errno = 0;
  REQUIRE(_access_s("no_such_u8_zzz.bin", 0) == ENOENT);
  REQUIRE(errno == ENOENT);

  // invalid modes -> EINVAL + errno
  errno = 0;
  REQUIRE(_access_s("u8acc_probe.txt", 7) == EINVAL);
  REQUIRE(errno == EINVAL);
  errno = 0;
  REQUIRE(_access_s("u8acc_probe.txt", -1) == EINVAL);
  REQUIRE(errno == EINVAL);

  // null path -> EINVAL (r15)
  errno = 0;
  REQUIRE(_access_s(nullptr, 0) == EINVAL);
  REQUIRE(errno == EINVAL);

  // read-only file + write probe -> EACCES
  _chmod("u8acc_probe.txt", _S_IREAD);
  errno = 0;
  REQUIRE(_access_s("u8acc_probe.txt", 2) == EACCES);
  REQUIRE(errno == EACCES);
  _chmod("u8acc_probe.txt", _S_IREAD | _S_IWRITE);

  // invalid UTF-8 bytes: the probe misses -> ENOENT (wine-anchored
  // shape: an unconvertible name never names a real file)
  errno = 0;
  REQUIRE(_access_s("\xff\xfe_bad", 0) == ENOENT);

  remove("u8acc_probe.txt");
}
