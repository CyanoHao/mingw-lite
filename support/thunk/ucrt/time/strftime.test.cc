#include <catch_amalgamated.hpp>

#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void set_tz(const char *value)
{
  char env[64];
  strcpy(env, "TZ=");
  strcat(env, value);
  putenv(env);
  _tzset();
}

/* 2026-09-27 12:34:56, Sunday, day 270 of the year */
static struct tm anchor(int isdst)
{
  struct tm t;
  memset(&t, 0, sizeof t);
  t.tm_year = 2026 - 1900;
  t.tm_mon = 8;
  t.tm_mday = 27;
  t.tm_hour = 12;
  t.tm_min = 34;
  t.tm_sec = 56;
  t.tm_wday = 0;
  t.tm_yday = 269;
  t.tm_isdst = isdst;
  return t;
}

/* 2026-09-27T12:34:56Z = 1790512496 (2026-01-01 = 1767225600,
 * +269 days, +45296 seconds) */
static const long long k_epoch_utc = 1790512496LL;

TEST_CASE("strftime C table snapshot")
{
  set_tz("UTC0");
  struct tm t = anchor(0);
  char buf[128];

  static const struct
  {
    const char *spec;
    const char *want;
  } cases[] = {
      {"%Y", "2026"},   {"%y", "26"},        {"%C", "20"},
      {"%m", "09"},     {"%d", "27"},        {"%e", "27"},
      {"%j", "270"},    {"%H", "12"},        {"%M", "34"},
      {"%S", "56"},     {"%I", "12"},        {"%p", "PM"},
      {"%a", "Sun"},    {"%A", "Sunday"},    {"%b", "Sep"},
      {"%h", "Sep"},    {"%B", "September"}, {"%u", "7"},
      {"%w", "0"},      {"%U", "39"},        {"%W", "38"},
      {"%V", "39"},     {"%G", "2026"},      {"%g", "26"},
      {"%D", "09/27/26"},                   {"%F", "2026-09-27"},
      {"%T", "12:34:56"},                   {"%R", "12:34"},
      {"%r", "12:34:56 PM"},                {"%x", "09/27/26"},
      {"%X", "12:34:56"},                   {"%c", "Sun Sep 27 12:34:56 2026"},
      {"%n", "\n"},     {"%t", "\t"},        {"%%", "%"},
      {"%z", "+0000"},  {"%Z", "UTC"},
  };

  for (const auto &c : cases) {
    size_t r = strftime(buf, sizeof buf, c.spec, &t);
    INFO("spec " << c.spec);
    REQUIRE(r == strlen(c.want));
    REQUIRE(strcmp(buf, c.want) == 0);
  }

  /* fixed epoch: UTC0 -> %s is the pure UTC conversion */
  size_t r = strftime(buf, sizeof buf, "%s", &t);
  char expect[32];
  sprintf(expect, "%lld", k_epoch_utc);
  REQUIRE(r == strlen(expect));
  REQUIRE(strcmp(buf, expect) == 0);
}

TEST_CASE("strftime padding and widths")
{
  set_tz("UTC0");
  struct tm t = anchor(0);
  char buf[128];

  t.tm_mday = 3;
  REQUIRE(strcmp((strftime(buf, sizeof buf, "%d", &t), buf), "03") == 0);
  REQUIRE(strcmp((strftime(buf, sizeof buf, "%e", &t), buf), " 3") == 0);
  REQUIRE(strcmp((strftime(buf, sizeof buf, "%_d", &t), buf), " 3") == 0);
  REQUIRE(strcmp((strftime(buf, sizeof buf, "%-d", &t), buf), "3") == 0);
  REQUIRE(strcmp((strftime(buf, sizeof buf, "%0d", &t), buf), "03") == 0);

  t.tm_mday = 27;
  /* musl width handling: the main-loop width pads with '0' after
   * trimming leading zeros (wine native rejects these outright) */
  REQUIRE(strcmp((strftime(buf, sizeof buf, "%5Y", &t), buf), "02026") == 0);
  REQUIRE(strcmp((strftime(buf, sizeof buf, "%5C", &t), buf), "00020") == 0);
  /* musl zeroes the format width for anything but C/F/G/Y, so %3e
   * degrades to plain %e: 27 already fills the default width 2 */
  REQUIRE(strcmp((strftime(buf, sizeof buf, "%3e", &t), buf), "27") == 0);
  /* year >= 10000 renders with the explicit plus */
  t.tm_year = 12345 - 1900;
  REQUIRE(strcmp((strftime(buf, sizeof buf, "%Y", &t), buf), "+12345") == 0);
}

TEST_CASE("strftime 12-hour clock")
{
  set_tz("UTC0");
  char buf[64];

  struct tm t = anchor(0);
  t.tm_hour = 0;
  REQUIRE(strcmp((strftime(buf, sizeof buf, "%I %p", &t), buf),
                 "12 AM") == 0);
  t.tm_hour = 12;
  REQUIRE(strcmp((strftime(buf, sizeof buf, "%I %p", &t), buf),
                 "12 PM") == 0);
  t.tm_hour = 18;
  REQUIRE(strcmp((strftime(buf, sizeof buf, "%I %p", &t), buf),
                 "06 PM") == 0);
  t.tm_hour = 23;
  REQUIRE(strcmp((strftime(buf, sizeof buf, "%I %p", &t), buf),
                 "11 PM") == 0);
}

TEST_CASE("strftime leap year")
{
  set_tz("UTC0");
  struct tm t = anchor(0);
  t.tm_year = 2024 - 1900;
  t.tm_mon = 1;
  t.tm_mday = 29;
  t.tm_yday = 59;
  char buf[64];
  /* %j is always zero-padded to width 3 */
  REQUIRE(strcmp((strftime(buf, sizeof buf, "%j", &t), buf), "060") == 0);
}

TEST_CASE("strftime z Z s TZ variants")
{
  char buf[128];
  char expect[32];
  struct tm t;

  set_tz("XXX-8");
  t = anchor(0);
  REQUIRE(strcmp((strftime(buf, sizeof buf, "%z|%Z", &t), buf),
                 "+0800|XXX") == 0);
  sprintf(expect, "%lld", k_epoch_utc - 28800);
  REQUIRE(strcmp((strftime(buf, sizeof buf, "%s", &t), buf), expect) == 0);
  /* no dst zone: isdst=1 has no dlt name and no dst shift */
  t.tm_isdst = 1;
  REQUIRE(strcmp((strftime(buf, sizeof buf, "%z", &t), buf), "+0800") == 0);
  REQUIRE(strcmp((strftime(buf, sizeof buf, "%Z", &t), buf), "") == 0);

  set_tz("EST5EDT");
  t = anchor(0);
  REQUIRE(strcmp((strftime(buf, sizeof buf, "%z|%Z", &t), buf),
                 "-0500|EST") == 0);
  /* %s = tm_to_secs - utc_offset(EST=-18000) → local + 5h */
  sprintf(expect, "%lld", k_epoch_utc + 18000);
  REQUIRE(strcmp((strftime(buf, sizeof buf, "%s", &t), buf), expect) == 0);

  t = anchor(1);
  REQUIRE(strcmp((strftime(buf, sizeof buf, "%z|%Z", &t), buf),
                 "-0400|EDT") == 0);
  /* utc_offset(EDT) = -14400 → local + 4h */
  sprintf(expect, "%lld", k_epoch_utc + 14400);
  REQUIRE(strcmp((strftime(buf, sizeof buf, "%s", &t), buf), expect) == 0);

  /* musl: unknown dst state renders %z/%Z empty; %s falls back to
   * the standard offset (our documented derivation) */
  t = anchor(-1);
  REQUIRE(strcmp((strftime(buf, sizeof buf, "%z", &t), buf), "") == 0);
  REQUIRE(strcmp((strftime(buf, sizeof buf, "%Z", &t), buf), "") == 0);
  sprintf(expect, "%lld", k_epoch_utc + 18000);
  REQUIRE(strcmp((strftime(buf, sizeof buf, "%s", &t), buf), expect) == 0);
}

TEST_CASE("strftime truncation contract")
{
  set_tz("UTC0");
  struct tm t = anchor(0);
  char buf[64];

  /* musl: partial content, return 0 (the NUL overwrites the last
   * byte); n=5 fits exactly and returns the length */
  memset(buf, 'A', sizeof buf);
  REQUIRE(strftime(buf, 4, "%Y", &t) == 0);
  REQUIRE(strcmp(buf, "202") == 0);
  REQUIRE(strftime(buf, 5, "%Y", &t) == 4);
  REQUIRE(strcmp(buf, "2026") == 0);

  memset(buf, 'A', sizeof buf);
  REQUIRE(strftime(buf, 4, "%Y-%m-%d", &t) == 0);

  /* unknown specifier: zero return, buffer NULed (l==0) */
  REQUIRE(strftime(buf, sizeof buf, "%q", &t) == 0);
  REQUIRE(buf[0] == 0);
  /* dangling %: the spec resolves to nothing, but the literal text
   * before it is already written — musl returns 0 with "abc" kept */
  REQUIRE(strftime(buf, sizeof buf, "abc%", &t) == 0);
  REQUIRE(strcmp(buf, "abc") == 0);

  /* E/O modifiers are consumed, not errors */
  REQUIRE(strcmp((strftime(buf, sizeof buf, "%EY|%Oy|%Ob", &t), buf),
                 "2026|26|Sep") == 0);

  /* zero count never touches the buffer */
  buf[0] = 'X';
  REQUIRE(strftime(buf, 0, "%Y", &t) == 0);
  REQUIRE(buf[0] == 'X');
}

TEST_CASE("strftime LC_TIME pollution adversarial")
{
  set_tz("UTC0");
  struct tm t = anchor(0);
  char buf[128];

  REQUIRE(setlocale(LC_TIME, "french") != nullptr);
  /* the native engine localizes these under LC_TIME; the overlay's
   * tables stay English by design (divergence recorded) */
  REQUIRE(strcmp((strftime(buf, sizeof buf, "%a %A %b %B %p", &t), buf),
                 "Sun Sunday Sep September PM") == 0);
  REQUIRE(strcmp((strftime(buf, sizeof buf, "%c", &t), buf),
                 "Sun Sep 27 12:34:56 2026") == 0);
  setlocale(LC_TIME, "C");
}

TEST_CASE("wcsftime engine")
{
  set_tz("UTC0");
  struct tm t = anchor(0);
  wchar_t wbuf[128];

  REQUIRE(wcsftime(wbuf, 128, L"%a %b %e %H:%M:%S %Y", &t) == 24);
  REQUIRE(wcscmp(wbuf, L"Sun Sep 27 12:34:56 2026") == 0);

  REQUIRE(wcsftime(wbuf, 128, L"%c %z %Z", &t) == 34);
  REQUIRE(wcscmp(wbuf, L"Sun Sep 27 12:34:56 2026 +0000 UTC") == 0);

  /* CJK literals pass through wide-side verbatim (1+4+1+2+1 = 9) */
  REQUIRE(wcsftime(wbuf, 128, L"\x5e74" L"%Y" L"\x6708" L"%d" L"\x65e5",
                   &t) == 9);
  REQUIRE(wcscmp(wbuf, L"\x5e74" L"2026" L"\x6708" L"27" L"\x65e5") == 0);

  /* wide truncation mirrors the musl wide contract (>= clamp):
   * n=4 on %Y copies then NULs the last position, returns 0 */
  wmemset(wbuf, L'A', 8);
  REQUIRE(wcsftime(wbuf, 4, L"%Y", &t) == 0);
  REQUIRE(wcscmp(wbuf, L"202") == 0);
  REQUIRE(wcsftime(wbuf, 5, L"%Y", &t) == 4);
  REQUIRE(wcscmp(wbuf, L"2026") == 0);

  set_tz("EST5EDT");
  t = anchor(1);
  REQUIRE(wcsftime(wbuf, 128, L"%z %Z", &t) == 9);
  REQUIRE(wcscmp(wbuf, L"-0400 EDT") == 0);
}

TEST_CASE("_strftime_l and _wcsftime_l shells")
{
  set_tz("UTC0");
  struct tm t = anchor(0);
  char buf[128];
  wchar_t wbuf[128];

  REQUIRE(_strftime_l(buf, sizeof buf, "%c", &t, nullptr) == 24);
  REQUIRE(strcmp(buf, "Sun Sep 27 12:34:56 2026") == 0);
  REQUIRE(_strftime_l(buf, sizeof buf, "%s", &t, nullptr) == 10);

  REQUIRE(_wcsftime_l(wbuf, 128, L"%A", &t, nullptr) == 6);
  REQUIRE(wcscmp(wbuf, L"Sunday") == 0);
}
