// M12 _utime32.
//
// The face is a plain wide delegation, so what is under test is that the
// delegation is faithful in both directions: the 32-bit __utimbuf32
// reaches _wutime32 untouched, and the caller's UTF-8 path becomes a
// wide path.  The wine anchors recorded in M12 pin down the edges that a
// 32/64 fold would have broken: 0xFFFFFFFF and 3000000000 are accepted
// rather than rejected, which is why the plan's original "reject past
// 2038 with EINVAL" premise was superseded (see plan-3 §4 M12).

#include <catch_amalgamated.hpp>

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/utime.h>
#include <time.h>

extern "C" int __cdecl _utime32(const char *, struct __utimbuf32 *);

namespace
{
  const char *const PATH = "test-utime32.tmp";

  int touch()
  {
    FILE *f = fopen(PATH, "wb");
    if (f == nullptr)
      return -1;
    fputs("x", f);
    fclose(f);
    return 0;
  }

  // __time32_t is a 32-bit signed long, so the past-2038 anchors are
  // bit patterns; they are compared unsigned so the test states the
  // 32-bit value rather than its sign extension.
  unsigned mtime32()
  {
    struct _stat32 st;
    if (_stat32(PATH, &st) != 0)
      return 0xFFFFFFFFu;
    return (unsigned)st.st_mtime;
  }
} // namespace

TEST_CASE("utime32: a 32-bit time round-trips through _stat32")
{
  REQUIRE(touch() == 0);

  struct __utimbuf32 times;
  times.actime = 1000000000;  // 2001-09-09
  times.modtime = 1234567890; // 2009-02-13

  REQUIRE(_utime32(PATH, &times) == 0);
  REQUIRE(mtime32() == 1234567890u);
  REQUIRE(times.actime == 1000000000);
  REQUIRE(times.modtime == 1234567890); // the caller's struct is untouched

  _unlink(PATH);
}

TEST_CASE("utime32: a null times pointer takes the current time")
{
  REQUIRE(touch() == 0);
  time_t before = time(nullptr);

  REQUIRE(_utime32(PATH, nullptr) == 0);

  long long after = (long long)(int)mtime32();
  REQUIRE(after >= (long long)before);
  REQUIRE(after <= (long long)before + 5);

  _unlink(PATH);
}

TEST_CASE("utime32: the 2106 edge follows the native mapping")
{
  REQUIRE(touch() == 0);

  // The plan's original premise was that a value past 2038 comes back
  // EINVAL.  wine accepts the call and the value sign-extends into the
  // 64-bit time the file-time conversion works in, so anything with the
  // top bit set lands before the epoch and clamps to 0.  The plain
  // delegation reproduces that by construction; these cases pin it down
  // so a future 32<->64 fold in this face would be caught.
  struct __utimbuf32 past = {0, (__time32_t)3000000000u};
  REQUIRE(_utime32(PATH, &past) == 0);
  REQUIRE(mtime32() == 0u);

  struct __utimbuf32 top = {0, (__time32_t)0x80000000u};
  REQUIRE(_utime32(PATH, &top) == 0);
  REQUIRE(mtime32() == 0u);

  struct __utimbuf32 minus2 = {0, (__time32_t)0xFFFFFFFEu};
  REQUIRE(_utime32(PATH, &minus2) == 0);
  REQUIRE(mtime32() == 0u);

  // the last value with the top bit clear is stored as itself
  struct __utimbuf32 edge = {0, (__time32_t)0x7FFFFFFFu};
  REQUIRE(_utime32(PATH, &edge) == 0);
  REQUIRE(mtime32() == 0x7FFFFFFFu);

  // and 2100000000, still under the 2106 ceiling, round-trips exactly
  struct __utimbuf32 near = {0, (__time32_t)2100000000L};
  REQUIRE(_utime32(PATH, &near) == 0);
  REQUIRE(mtime32() == 2100000000u);

  _unlink(PATH);
}

TEST_CASE("utime32: a -1 member does not disturb the other")
{
  // -1 is applied to its own member rather than meaning "leave this one
  // alone", so the value that lands on the other side is unchanged
  REQUIRE(touch() == 0);

  struct __utimbuf32 at = {(__time32_t)0xFFFFFFFFu, 3000000};
  REQUIRE(_utime32(PATH, &at) == 0);
  REQUIRE(mtime32() == 3000000u);

  struct __utimbuf32 mt = {3000000, (__time32_t)0xFFFFFFFFu};
  REQUIRE(_utime32(PATH, &mt) == 0);
  REQUIRE(mtime32() == 3000000u);

  // both members -1 is the documented "do not change" pair, but wine
  // applies it anyway and lands on a time of its own making, so only
  // the success is asserted here -- what matters for this face is that
  // the pair is not folded or rejected on the way to the wide native
  struct __utimbuf32 nochange = {(__time32_t)0xFFFFFFFFu,
                                 (__time32_t)0xFFFFFFFFu};
  REQUIRE(_utime32(PATH, &nochange) == 0);

  _unlink(PATH);
}

TEST_CASE("utime32: the zero epoch is a real time")
{
  REQUIRE(touch() == 0);

  struct __utimbuf32 zero = {0, 0};
  REQUIRE(_utime32(PATH, &zero) == 0);
  REQUIRE(mtime32() == 0u);

  _unlink(PATH);
}

TEST_CASE("utime32: error paths")
{
  errno = 0;
  REQUIRE(_utime32(nullptr, nullptr) == -1);
  REQUIRE(errno == EINVAL);

  errno = 0;
  REQUIRE(_utime32("no-such-file-4f2a.tmp", nullptr) == -1);
  REQUIRE(errno == ENOENT);

  _unlink(PATH);
}

TEST_CASE("utime32: a UTF-8 path reaches the wide native")
{
  // the transcoded path is the only thing the face does, so a name that
  // is not representable in the ANSI code page proves the wide call
  const char *utf8 = "\xE6\xB5\x8B\xE8\xAF\x95\x2D\x75\x74\x69\x6D\x65\x33"
                     "\x32\x2E\x74\x6D\x70"; // 测试-utime32.tmp
  FILE *f = fopen(utf8, "wb");
  REQUIRE(f != nullptr);
  fclose(f);

  struct __utimbuf32 times = {1000000, 2000000};
  REQUIRE(_utime32(utf8, &times) == 0);

  struct _stat32 st;
  REQUIRE(_stat32(utf8, &st) == 0);
  REQUIRE((unsigned)st.st_mtime == 2000000u);

  _unlink(utf8);
}
