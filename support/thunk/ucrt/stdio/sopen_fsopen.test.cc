#include <catch_amalgamated.hpp>

#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <share.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

extern "C" int __cdecl _sopen_s(int *fd, const char *path, int oflag,
                                int shflag, int pmode);
extern "C" int __cdecl _sopen_dispatch(const char *path, int oflag,
                                       int shflag, int pmode, int *fd, ...);
extern "C" int __cdecl freopen_s(FILE **pFile, const char *path,
                                 const char *mode, FILE *stream);

TEST_CASE("_fsopen")
{
  const char path[] = "test-fsopen-\xe4\xbd\xa0.txt"; // 你

  FILE *fp = _fsopen(path, "wb", _SH_DENYRW);
  REQUIRE(fp != nullptr);
  const char ni[] = "\xe4\xbd\xa0"; // 你
  REQUIRE(fwrite(ni, 1, sizeof ni, fp) == sizeof ni);
  REQUIRE(fclose(fp) == 0);

  // while the share lock is gone, reopening must work; probe the
  // denial shape with a fresh DENYRW handle first
  fp = _fsopen(path, "rb", _SH_DENYRW);
  REQUIRE(fp != nullptr);
  errno = 0;
  FILE *conflict = _fsopen(path, "rb", _SH_DENYNO);
  REQUIRE(conflict == nullptr);
  REQUIRE(errno == EACCES);
  REQUIRE(fclose(fp) == 0);

  fp = _fsopen(path, "rb", _SH_DENYNO);
  REQUIRE(fp != nullptr);
  char buf[8];
  REQUIRE(fread(buf, 1, sizeof ni, fp) == sizeof ni);
  REQUIRE(memcmp(buf, ni, sizeof ni) == 0);
  REQUIRE(fclose(fp) == 0);

  REQUIRE(remove(path) == 0);
}

TEST_CASE("_sopen_s")
{
  const char path[] = "test-sopen-\xe4\xbd\xa0.bin";

  int fd = -1;
  errno = 0;
  REQUIRE(_sopen_s(&fd, path, _O_WRONLY | _O_CREAT | _O_TRUNC, _SH_DENYRW,
                   _S_IREAD | _S_IWRITE) == 0);
  REQUIRE(fd >= 0);
  const char ni[] = "\xe4\xbd\xa0";
  REQUIRE(_write(fd, ni, (int)sizeof ni) == (int)sizeof ni);
  REQUIRE(_close(fd) == 0);

  // share violation returns the errno code directly (wine anchor: 13)
  fd = -1;
  errno = 0;
  REQUIRE(_sopen_s(&fd, path, _O_WRONLY | _O_CREAT | _O_TRUNC, _SH_DENYRW,
                   _S_IREAD | _S_IWRITE) == 0);
  REQUIRE(fd >= 0);
  int fd2 = 99;
  errno = 0;
  REQUIRE(_sopen_s(&fd2, path, _O_RDONLY, _SH_DENYNO, 0) == EACCES);
  REQUIRE(errno == EACCES);
  REQUIRE(fd2 == -1);
  REQUIRE(_close(fd) == 0);

  // wine anchors: null fd -> EINVAL; null path -> EINVAL with *fd
  // pre-set to -1
  errno = 0;
  REQUIRE(_sopen_s(nullptr, path, _O_RDONLY, _SH_DENYNO, 0) == EINVAL);
  REQUIRE(errno == EINVAL);
  fd2 = 99;
  errno = 0;
  REQUIRE(_sopen_s(&fd2, nullptr, _O_RDONLY, _SH_DENYNO, 0) == EINVAL);
  REQUIRE(errno == EINVAL);
  REQUIRE(fd2 == -1);

  REQUIRE(remove(path) == 0);
}

TEST_CASE("_sopen_dispatch")
{
  const char path[] = "test-dispatch-\xe4\xbd\xa0.bin";

  int fd = -1;
  errno = 0;
  REQUIRE(_sopen_dispatch(path, _O_WRONLY | _O_CREAT | _O_TRUNC,
                          _SH_DENYNO, _S_IREAD | _S_IWRITE,
                          &fd) == 0);
  REQUIRE(fd >= 0);
  REQUIRE(_close(fd) == 0);

  // wine anchors: null path -> EINVAL with *fd = -1; null fd -> EINVAL
  fd = 99;
  errno = 0;
  REQUIRE(_sopen_dispatch(nullptr, _O_RDONLY, _SH_DENYNO, 0, &fd) == EINVAL);
  REQUIRE(errno == EINVAL);
  REQUIRE(fd == -1);

  errno = 0;
  REQUIRE(_sopen_dispatch(path, _O_RDONLY, _SH_DENYNO, 0, nullptr) == EINVAL);
  REQUIRE(errno == EINVAL);

  REQUIRE(remove(path) == 0);
}

TEST_CASE("freopen_s")
{
  const char path[] = "test-freopen-\xe4\xbd\xa0.txt";
  const char path2[] = "test-freopen2-\xe4\xbd\xa0.txt";

  FILE *fp = fopen(path, "wb");
  REQUIRE(fp != nullptr);

  // success: the SAME stream is reused (wine anchor)
  FILE *fp2 = (FILE *)1;
  errno = 0;
  REQUIRE(freopen_s(&fp2, path2, "w", fp) == 0);
  REQUIRE(fp2 == fp);
  const char ni[] = "\xe4\xbd\xa0";
  REQUIRE(fwrite(ni, 1, sizeof ni, fp2) == sizeof ni);
  REQUIRE(fclose(fp2) == 0);

  // missing file: errno code + nulled out-pointer (fopen_s shape).
  // The stream is consumed by the failed reopen (freopen closes it
  // even on failure) — do not touch it afterwards.
  fp = fopen(path2, "rb");
  REQUIRE(fp != nullptr);
  fp2 = (FILE *)1;
  errno = 0;
  REQUIRE(freopen_s(&fp2, "no-such-u8crt-dir/x", "r", fp) == ENOENT);
  REQUIRE(errno == ENOENT);
  REQUIRE(fp2 == nullptr);

  // validation failures: EINVAL, *pFile untouched (wine anchor —
  // unlike fopen_s there is no pre-nulling here)
  fp2 = (FILE *)1;
  errno = 0;
  REQUIRE(freopen_s(nullptr, path2, "r", fp) == EINVAL);
  REQUIRE(errno == EINVAL);
  fp2 = (FILE *)1;
  errno = 0;
  REQUIRE(freopen_s(&fp2, nullptr, "r", fp) == EINVAL);
  REQUIRE(fp2 == (FILE *)1);
  fp2 = (FILE *)1;
  errno = 0;
  REQUIRE(freopen_s(&fp2, path2, nullptr, fp) == EINVAL);
  REQUIRE(fp2 == (FILE *)1);
  fp2 = (FILE *)1;
  errno = 0;
  REQUIRE(freopen_s(&fp2, path2, "r", nullptr) == EINVAL);
  REQUIRE(fp2 == (FILE *)1);

  REQUIRE(remove(path) == 0);
  REQUIRE(remove(path2) == 0);
}
