#include <catch_amalgamated.hpp>

#include <errno.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <wchar.h>

#include <string>

// Engine-level test (musl layer): the IAT-level shell routing arrives with
// M2; the console channel is exercised separately.
#include "../../utf8-musl/internal/stdio_impl.h"
#include <thunk/u8crt/musl.h>

namespace
{
  namespace musl = mingw_thunk::musl;

  std::string g_out;

  // write-fn contract (mirrors __stdio_write): emit pending [wbase, wpos)
  // first, then the new chunk, then reset the buffer pointers.
  size_t cap_write(musl::FILE *fp, const unsigned char *s, size_t l)
  {
    g_out.append(reinterpret_cast<const char *>(fp->wbase),
                 static_cast<size_t>(fp->wpos - fp->wbase));
    if (l)
      g_out.append(reinterpret_cast<const char *>(s), l);
    fp->wpos = fp->wbase = fp->buf;
    return l;
  }

  struct engine
  {
    unsigned char buf[128];
    musl::FILE f;

    engine()
    {
      f = {};
      f.write = &cap_write;
      f.buf = buf;
      f.buf_size = sizeof buf;
      f.lock = -1;
      f.lbf = -1;
      g_out.clear();
    }

    int render(const char *fmt, ...)
    {
      va_list ap;
      va_start(ap, fmt);
      int r = musl::vfprintf(&f, fmt, ap);
      va_end(ap);
      return r;
    }

    std::string take()
    {
      g_out.append(reinterpret_cast<const char *>(f.wbase),
                   static_cast<size_t>(f.wpos - f.wbase));
      f.wpos = f.wbase = f.buf;
      std::string out;
      out.swap(g_out);
      return out;
    }
  };

  struct exp_digits_guard
  {
    int saved;

    explicit exp_digits_guard(int v) : saved(musl::exp_digits_min)
    {
      musl::exp_digits_min = v;
    }

    ~exp_digits_guard()
    {
      musl::exp_digits_min = saved;
    }
  };
} // namespace

TEST_CASE("musl vfprintf %ls surrogate pairs")
{
  SECTION("BMP and non-BMP exact bytes")
  {
    engine e;
    REQUIRE(e.render("%ls", L"你aé😀𝄞") == (int)strlen("你aé😀𝄞"));
    REQUIRE(e.take() == "你aé😀𝄞");
  }

  SECTION("pairs at start, middle and end")
  {
    engine e;
    REQUIRE(e.render("%ls", L"😀中𝄞") == (int)strlen("😀中𝄞"));
    REQUIRE(e.take() == "😀中𝄞");
  }

  SECTION("no precision measures the whole string")
  {
    engine e;
    REQUIRE(e.render("[%ls]", L"你好") == (int)strlen("[你好]"));
    REQUIRE(e.take() == "[你好]");
  }

  SECTION("width and left-adjust")
  {
    engine e;
    REQUIRE(e.render("[%8ls]", L"😀") == (int)strlen("[    😀]"));
    REQUIRE(e.take() == "[    😀]");

    REQUIRE(e.render("[%-8ls]", L"😀") == (int)strlen("[😀    ]"));
    REQUIRE(e.take() == "[😀    ]");
  }

  SECTION("precision never splits a pair")
  {
    engine e;
    REQUIRE(e.render("%.1ls", L"😀x") == 0);
    REQUIRE(e.take().empty());

    REQUIRE(e.render("%.3ls", L"😀x") == 0);
    REQUIRE(e.take().empty());

    REQUIRE(e.render("%.4ls", L"😀x") == (int)strlen("😀"));
    REQUIRE(e.take() == "😀");

    REQUIRE(e.render("%.5ls", L"😀x") == (int)strlen("😀x"));
    REQUIRE(e.take() == "😀x");
  }

  SECTION("precision counts bytes for BMP too")
  {
    engine e;
    REQUIRE(e.render("%.1ls", L"你") == 0);
    REQUIRE(e.take().empty());

    REQUIRE(e.render("%.3ls", L"你") == (int)strlen("你"));
    REQUIRE(e.take() == "你");
  }

  SECTION("lone high surrogate fails")
  {
    const wchar_t s[] = {0xd83d, 0};
    engine e;
    errno = 0;
    REQUIRE(e.render("%ls", s) == -1);
    REQUIRE(errno == EILSEQ);
  }

  SECTION("lone low surrogate fails")
  {
    const wchar_t s[] = {0xde00, 0};
    engine e;
    errno = 0;
    REQUIRE(e.render("%ls", s) == -1);
    REQUIRE(errno == EILSEQ);
  }

  SECTION("high surrogate before NUL fails")
  {
    const wchar_t s[] = {0xd83d, 0x4f60, 0};
    engine e;
    errno = 0;
    REQUIRE(e.render("%ls", s) == -1);
    REQUIRE(errno == EILSEQ);
  }

  SECTION("mixed long string matches the narrow rendering")
  {
    engine e;
    REQUIRE(e.render("%ls-%s-%ls",
                     L"你好😀世界𝄞end",
                     "middle€text",
                     L"Привет👀") ==
            (int)strlen("你好😀世界𝄞end-middle€text-Привет👀"));
    REQUIRE(e.take() == "你好😀世界𝄞end-middle€text-Привет👀");
  }

  SECTION("%lc BMP passes through")
  {
    engine e;
    REQUIRE(e.render("[%lc]", L'你') == (int)strlen("[你]"));
    REQUIRE(e.take() == "[你]");
  }

  SECTION("%lc with a lone surrogate argument fails")
  {
    engine e;
    errno = 0;
    REQUIRE(e.render("%lc", (wchar_t)0xd83d) == -1);
    REQUIRE(errno == EILSEQ);
  }

  SECTION("zero precision")
  {
    engine e;
    REQUIRE(e.render("%.0ls", L"你好") == 0);
    REQUIRE(e.take().empty());
  }
}

TEST_CASE("musl vfprintf exponent digit width")
{
  SECTION("default is two digits")
  {
    engine e;
    REQUIRE(e.render("%e", 1.5) == 12);
    REQUIRE(e.take() == "1.500000e+00");

    REQUIRE(e.render("%.2e", 0.000015) == 8);
    REQUIRE(e.take() == "1.50e-05");
  }

  SECTION("large exponents stay unclamped")
  {
    engine e;
    REQUIRE(e.render("%e", 1e100) == (int)strlen("1.000000e+100"));
    REQUIRE(e.take() == "1.000000e+100");
  }

  SECTION("three-digit mode via exp_digits_min")
  {
    exp_digits_guard guard(3);
    engine e;
    REQUIRE(e.render("%e", 1.5) == 13);
    REQUIRE(e.take() == "1.500000e+000");

    REQUIRE(e.render("%.2e", 0.000015) == 9);
    REQUIRE(e.take() == "1.50e-005");

    REQUIRE(e.render("%g", 1e-5) == 6);
    REQUIRE(e.take() == "1e-005");

    REQUIRE(e.render("%E", 1234.5) == (int)strlen("1.234500E+003"));
    REQUIRE(e.take() == "1.234500E+003");

    REQUIRE(e.render("%e", 1e100) == (int)strlen("1.000000e+100"));
    REQUIRE(e.take() == "1.000000e+100");
  }

  SECTION("exp_digits_min is restored to two digits")
  {
    {
      exp_digits_guard guard(3);
    }
    engine e;
    REQUIRE(e.render("%e", 1.5) == 12);
    REQUIRE(e.take() == "1.500000e+00");
  }
}
