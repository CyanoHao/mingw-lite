#include <catch_amalgamated.hpp>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>

typedef int errno_t;

extern "C" int *__cdecl __daylight(void);
extern "C" long *__cdecl __dstbias(void);
extern "C" long *__cdecl __timezone(void);
extern "C" char **__cdecl __tzname(void);
extern "C" errno_t __cdecl _get_tzname(size_t *, char *, size_t, int);

static void set_tz(const char *value)
{
  char env[64];
  strcpy(env, "TZ=");
  strcat(env, value);
  putenv(env);
  _tzset();
}

TEST_CASE("TZ POSIX subset")
{
  set_tz("XXX-8");
  REQUIRE(*__timezone() == -28800);
  REQUIRE(*__daylight() == 0);
  REQUIRE(*__dstbias() == 0);
  REQUIRE(strcmp(__tzname()[0], "XXX") == 0);
  REQUIRE(__tzname()[1][0] == 0);

  set_tz("EST5EDT");
  REQUIRE(*__timezone() == 18000);
  REQUIRE(*__daylight() == 1);
  REQUIRE(*__dstbias() == -3600);
  REQUIRE(strcmp(__tzname()[0], "EST") == 0);
  REQUIRE(strcmp(__tzname()[1], "EDT") == 0);

  set_tz("UTC0");
  REQUIRE(*__timezone() == 0);
  REQUIRE(*__daylight() == 0);
  REQUIRE(strcmp(__tzname()[0], "UTC") == 0);

  set_tz("GMT+2");
  REQUIRE(*__timezone() == 7200);
  REQUIRE(strcmp(__tzname()[0], "GMT") == 0);

  set_tz("ZZZ+3:30");
  REQUIRE(*__timezone() == 12600);

  /* explicit dst offset stores the difference from std */
  set_tz("AAA4BBB6");
  REQUIRE(*__timezone() == 4 * 3600);
  REQUIRE(*__daylight() == 1);
  REQUIRE(*__dstbias() == 2 * 3600);
  REQUIRE(strcmp(__tzname()[1], "BBB") == 0);
}

TEST_CASE("TZ invalid falls back to UTC")
{
  /* wine anchor: TZ=bogus -> timezone 0; ours names it UTC */
  set_tz("bogus");
  REQUIRE(*__timezone() == 0);
  REQUIRE(*__daylight() == 0);
  REQUIRE(*__dstbias() == 0);
  REQUIRE(strcmp(__tzname()[0], "UTC") == 0);
  REQUIRE(__tzname()[1][0] == 0);

  set_tz("12"); /* no alpha name */
  REQUIRE(*__timezone() == 0);
}

TEST_CASE("TZ unset follows the registry oracle")
{
  /* wine anchor 2026-09-27: msvcrt _putenv("TZ") only drops the CRT
   * copy — the Win32 variable (which this engine reads) survives — so
   * clear it there directly */
  _putenv("TZ");
  SetEnvironmentVariableA("TZ", NULL);
  _tzset();

  TIME_ZONE_INFORMATION tzi;
  REQUIRE(GetTimeZoneInformation(&tzi) != TIME_ZONE_ID_INVALID);

  REQUIRE(*__timezone() ==
          ((long)tzi.Bias + (long)tzi.StandardBias) * 60);
  REQUIRE(*__daylight() == (tzi.DaylightDate.wMonth != 0));
  REQUIRE(*__dstbias() ==
          ((tzi.DaylightDate.wMonth != 0) ? (long)tzi.DaylightBias * 60
                                         : 0));

  char expect[64];
  REQUIRE(WideCharToMultiByte(CP_UTF8, 0, tzi.StandardName, -1, expect,
                              sizeof expect, NULL, NULL) > 0);
  REQUIRE(strcmp(__tzname()[0], expect) == 0);
  REQUIRE(WideCharToMultiByte(CP_UTF8, 0, tzi.DaylightName, -1, expect,
                              sizeof expect, NULL, NULL) > 0);
  REQUIRE(strcmp(__tzname()[1], expect) == 0);
}

TEST_CASE("tz globals pointer stability")
{
  set_tz("EST5EDT");
  int *pday = __daylight();
  long *pdst = __dstbias();
  long *ptz = __timezone();
  char **pname = __tzname();

  set_tz("XXX-8");
  REQUIRE(__daylight() == pday);
  REQUIRE(__dstbias() == pdst);
  REQUIRE(__timezone() == ptz);
  REQUIRE(__tzname() == pname);
  REQUIRE(*pday == 0); /* same storage, new value */
  REQUIRE(strcmp(pname[0], "XXX") == 0);
}

TEST_CASE("_get_tzname")
{
  set_tz("EST5EDT");

  size_t secs = 0xdeadbeef;
  char buf[64];
  errno_t rc;

  /* success: *retval = required bytes including the NUL (wine anchor:
   * "PST" -> 4), not an offset in seconds */
  memset(buf, 'A', sizeof buf);
  rc = _get_tzname(&secs, buf, sizeof buf, 0);
  REQUIRE(rc == 0);
  REQUIRE(secs == 4);
  REQUIRE(strcmp(buf, "EST") == 0);

  rc = _get_tzname(&secs, buf, sizeof buf, 1);
  REQUIRE(rc == 0);
  REQUIRE(secs == 4);
  REQUIRE(strcmp(buf, "EDT") == 0);

  /* one byte short -> ERANGE (34) + reset, errno untouched */
  errno = 0;
  memset(buf, 'A', sizeof buf);
  rc = _get_tzname(&secs, buf, 3, 0);
  REQUIRE(rc == 34);
  REQUIRE(buf[0] == 0);
  REQUIRE(errno == 0);

  /* exact fit */
  rc = _get_tzname(&secs, buf, 4, 0);
  REQUIRE(rc == 0);
  REQUIRE(strcmp(buf, "EST") == 0);

  /* size 0: ERANGE, buffer untouched (conservative unprobed edge) */
  memset(buf, 'A', sizeof buf);
  rc = _get_tzname(&secs, buf, 0, 0);
  REQUIRE(rc == 34);
  REQUIRE((unsigned char)buf[0] == 'A');

  /* null buffer -> EINVAL (wine anchor) */
  errno = 0;
  rc = _get_tzname(&secs, NULL, sizeof buf, 0);
  REQUIRE(rc == EINVAL);
  REQUIRE(errno == EINVAL);

  /* null retval -> EINVAL */
  rc = _get_tzname(NULL, buf, sizeof buf, 0);
  REQUIRE(rc == EINVAL);

  /* index outside {0,1} -> EINVAL, out-params untouched (anchor) */
  secs = 0xdeadbeef;
  memset(buf, 'A', sizeof buf);
  rc = _get_tzname(&secs, buf, sizeof buf, 99);
  REQUIRE(rc == EINVAL);
  REQUIRE(secs == 0xdeadbeef);
  REQUIRE((unsigned char)buf[0] == 'A');
}

TEST_CASE("%Z and __tzname share the source")
{
  set_tz("XXX-8");
  struct tm t;
  memset(&t, 0, sizeof t);
  t.tm_year = 2026 - 1900;
  t.tm_mon = 8;
  t.tm_mday = 27;
  t.tm_isdst = 0;

  char buf[64];
  REQUIRE(strftime(buf, sizeof buf, "%Z", &t) == strlen(__tzname()[0]));
  REQUIRE(strcmp(buf, __tzname()[0]) == 0);
}
