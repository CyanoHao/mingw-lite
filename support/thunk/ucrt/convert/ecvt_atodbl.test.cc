#include <catch_amalgamated.hpp>

#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

static void check_ecvt(double v, int n, const char *digits, int dec, int sign)
{
  int d = -99, s = -99;
  errno = 0;
  char *r = _ecvt(v, n, &d, &s);
  REQUIRE(r != nullptr);
  REQUIRE(strcmp(r, digits) == 0);
  REQUIRE(d == dec);
  REQUIRE(s == sign);
  REQUIRE(errno == 0);
}

static void check_fcvt(double v, int n, const char *digits, int dec, int sign)
{
  int d = -99, s = -99;
  errno = 0;
  char *r = _fcvt(v, n, &d, &s);
  REQUIRE(r != nullptr);
  REQUIRE(strcmp(r, digits) == 0);
  REQUIRE(d == dec);
  REQUIRE(s == sign);
  REQUIRE(errno == 0);
}

static void check_gcvt(double v, int p, const char *out)
{
  char buf[128];
  errno = 0;
  char *r = _gcvt(v, p, buf);
  REQUIRE(r == buf);
  REQUIRE(strcmp(buf, out) == 0);
  REQUIRE(errno == 0);
}

TEST_CASE("_ecvt snapshots (wine C group)")
{
  check_ecvt(123.456, 5, "12346", 3, 0);
  check_ecvt(0.999, 5, "99900", 0, 0); // no carry: plain rounding
  check_ecvt(-123.456, 5, "12346", 3, 1);
  check_ecvt(123.456, 0, "", 3, 0);
  check_ecvt(0.0, 5, "00000", 0, 0);
  check_ecvt(-0.0, 5, "00000", 0, 1);
  check_ecvt(1e-5, 8, "10000000", -4, 0);
  check_ecvt(9.999, 2, "10", 2, 0); // rounding carry moves decpt
  check_ecvt(0.5678, 3, "568", 0, 0);
  check_ecvt(1.0 / 3.0, 17, "33333333333333331", 0, 0);
}

TEST_CASE("_fcvt snapshots (wine C2 group)")
{
  check_fcvt(123.456, 2, "12346", 3, 0);
  check_fcvt(1.005, 2, "100", 1, 0); // 1.00499... rounds to 1.00
  check_fcvt(0.005, 2, "1", -1, 0); // 0.01 with the leading zero stripped
  check_fcvt(-1.005, 3, "1005", 1, 1);
  check_fcvt(123.456, 0, "123", 3, 0);
  check_fcvt(1e20, 3, "100000000000000000000000", 21, 0);
  check_fcvt(1e-8, 10, "100", -7, 0);
  check_fcvt(0.0, 3, "000", 0, 0); // zero kept verbatim
  check_fcvt(-0.0, 2, "00", 0, 1);
  check_fcvt(100.5, 2, "10050", 3, 0);
}

TEST_CASE("_gcvt snapshots (wine C3 group — printf %g shape)")
{
  check_gcvt(1e-5, 4, "1e-05"); // %g threshold: exponent < -4 -> E style
  check_gcvt(0.0001, 4, "0.0001"); // exponent -4 stays F style
  check_gcvt(1.5, 6, "1.5");
  check_gcvt(123456.0, 6, "123456"); // magnitude == precision - 1: F
  check_gcvt(1234567.0, 6, "1.23457e+06"); // >= precision: E
  check_gcvt(-0.5, 4, "-0.5");
  check_gcvt(1.0 / 3.0, 6, "0.333333");
  check_gcvt(3.14159265358979, 12, "3.14159265359");
  check_gcvt(1e300, 5, "1e+300");
  check_gcvt(1e-300, 10, "1e-300");
  check_gcvt(0.0, 5, "0");
  check_gcvt(-1.0, 17, "-1");
  check_gcvt(12345.6, 8, "12345.6");
}

TEST_CASE("_ecvt_s capacity matrix (wine C4 + r6-r8, r14)")
{
  char buf[64];
  int dec = -99, sign = -99;

  errno = 0;
  REQUIRE(_ecvt_s(buf, 7, 123.456, 5, &dec, &sign) == 0); // count + 2
  REQUIRE(strcmp(buf, "12346") == 0);
  REQUIRE(dec == 3);
  REQUIRE(sign == 0);
  REQUIRE(errno == 0);

  memset(buf, '#', 6), buf[6] = 0;
  dec = -99; // re-arm: untouched-means-previous-value, sentinel must be
  sign = -99; // fresh (the wine probe used fresh variables)
  errno = 0;
  REQUIRE(_ecvt_s(buf, 6, 123.456, 5, &dec, &sign) == ERANGE);
  REQUIRE(errno == ERANGE);
  REQUIRE(buf[0] == '#'); // wine: buffer untouched on ERANGE
  REQUIRE(dec == -99); // out-params untouched too
  REQUIRE(sign == -99);

  errno = 0;
  REQUIRE(_ecvt_s(buf, 3, 1.5, 3, &dec, &sign) == ERANGE); // r14 graceful

  errno = 1234;
  REQUIRE(_ecvt_s(nullptr, 10, 1.5, 3, &dec, &sign) == EINVAL); // r8
  REQUIRE(errno == 1234); // EINVAL paths leave errno alone
  REQUIRE(_ecvt_s(buf, 10, 1.5, 3, nullptr, &sign) == EINVAL); // r6
  REQUIRE(_ecvt_s(buf, 10, 1.5, 3, &dec, nullptr) == EINVAL); // r7

  errno = 0;
  REQUIRE(_ecvt_s(buf, 0, 1.5, 3, &dec, &sign) == ERANGE); // cap 0

  REQUIRE(_ecvt_s(buf, sizeof(buf), 123.456, 0, &dec, &sign) == 0);
  REQUIRE(buf[0] == 0);
  REQUIRE(dec == 3);
}

TEST_CASE("_fcvt_s capacity matrix (wine probe2)")
{
  char buf[64];
  int dec = -99, sign = -99;

  errno = 0;
  REQUIRE(_fcvt_s(buf, 4, 1.005, 2, &dec, &sign) == 0); // digits + 1
  REQUIRE(strcmp(buf, "100") == 0);
  REQUIRE(dec == 1);

  errno = 0;
  REQUIRE(_fcvt_s(buf, 3, 1.005, 2, &dec, &sign) == ERANGE);
  REQUIRE(errno == ERANGE); // graceful (wine shell dies here — noted)

  errno = 1234;
  REQUIRE(_fcvt_s(nullptr, 10, 1.5, 3, &dec, &sign) == EINVAL); // r13
  REQUIRE(errno == 1234);
}

TEST_CASE("_gcvt_s capacity matrix (wine probe2 + r9/r10)")
{
  char buf[64];

  errno = 0;
  REQUIRE(_gcvt_s(buf, 7, 1.5, 6) == 0); // precision < cap
  REQUIRE(strcmp(buf, "1.5") == 0);

  errno = 0;
  memset(buf, '#', 4), buf[4] = 0;
  REQUIRE(_gcvt_s(buf, 4, 1.5, 6) == ERANGE); // precision >= cap
  REQUIRE(errno == ERANGE);
  REQUIRE(buf[0] == 0); // cleared (unlike _ecvt_s)

  errno = 0;
  REQUIRE(_gcvt_s(buf, 9, 1234567.0, 6) == ERANGE); // rendered too long
  REQUIRE(errno == ERANGE);
  REQUIRE(buf[0] == 0);

  errno = 0;
  buf[0] = '#';
  REQUIRE(_gcvt_s(buf, 0, 1.5, 6) == ERANGE); // r10: untouched at cap 0
  REQUIRE(errno == ERANGE);
  REQUIRE((unsigned char)buf[0] == '#');

  errno = 1234;
  REQUIRE(_gcvt_s(nullptr, 10, 1.5, 6) == EINVAL); // r9
  REQUIRE(errno == 1234);
}

TEST_CASE("_atodbl family struct fills (wine D group)")
{
  _CRT_DOUBLE d;
  _CRT_FLOAT f;
  _LDOUBLE ld;

  errno = 0;
  REQUIRE(_atodbl(&d, (char *)"3.5") == 0);
  REQUIRE(d.x == 3.5);
  REQUIRE(errno == 0);

  REQUIRE(_atodbl(&d, (char *)"1e999") == 3); // _OVERFLOW
  REQUIRE(isinf(d.x));
  REQUIRE(errno == 0); // errno never touched by this family

  REQUIRE(_atodbl(&d, (char *)"inf") == 3); // parsed inf reports overflow
  REQUIRE(_atodbl(&d, (char *)"1e-999") == 4); // _UNDERFLOW
  REQUIRE(d.x == 0.0);
  REQUIRE(_atodbl(&d, (char *)"abc") == 0);
  REQUIRE(d.x == 0.0);
  REQUIRE(_atodbl(&d, (char *)"") == 0);
  REQUIRE(_atodbl(&d, (char *)"3abc") == 0);
  REQUIRE(d.x == 3.0);
  REQUIRE(_atodbl(&d, (char *)"  -2.5e2") == 0);
  REQUIRE(d.x == -250.0);
  REQUIRE(_atodbl(&d, (char *)"0x1.8p3") == 0);
  REQUIRE(d.x == 12.0);

  errno = 0;
  REQUIRE(_atoflt(&f, (char *)"3.5") == 0);
  REQUIRE(f.f == 3.5f);
  REQUIRE(_atoflt(&f, (char *)"1e999") == 3);
  REQUIRE(isinf(f.f));

  errno = 0;
  REQUIRE(_atoldbl(&ld, (char *)"3.5") == 0);
  long double expect = 3.5L;
  REQUIRE(memcmp(ld.ld, &expect, 10) == 0); // true 80-bit fill
  REQUIRE(_atoldbl(&ld, (char *)"0.1") == 0);
  expect = 0.1L;
  REQUIRE(memcmp(ld.ld, &expect, 10) == 0);

  // _l shells ignore the locale argument
  REQUIRE(_atodbl_l(&d, (char *)"2.25", nullptr) == 0);
  REQUIRE(d.x == 2.25);
  REQUIRE(_atoflt_l(&f, (char *)"2.25", nullptr) == 0);
  REQUIRE(f.f == 2.25f);
  REQUIRE(_atoldbl_l(&ld, (char *)"2.25", nullptr) == 0);
  expect = 2.25L;
  REQUIRE(memcmp(ld.ld, &expect, 10) == 0);

  // null pointers: graceful EINVAL (native r3/r4/r5/r15 crash — not
  // replicated; plan-3 §3.7 divergence note)
  errno = 0;
  REQUIRE(_atodbl(nullptr, (char *)"3") == EINVAL);
  REQUIRE(_atoflt(nullptr, (char *)"3") == EINVAL);
  REQUIRE(_atoldbl(nullptr, (char *)"3") == EINVAL);
  REQUIRE(errno == EINVAL);
  REQUIRE(_atodbl(&d, nullptr) == EINVAL);
}
