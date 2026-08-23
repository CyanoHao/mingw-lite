// M12 conio family: the internal channel, driven headlessly.
//
// The device is a redirected file under wine, so WriteConsoleW and
// ReadConsoleW both fail with ERROR_INVALID_HANDLE.  That is the wine
// anchor every native face reproduces, and it means the testable half
// of the channel is the part that does not touch the device: the render
// (options matrix, UTF-8 and UTF-16 payloads), the _putch UTF-8 park,
// the _cgets conversion protocol and the argument gates.  The device
// half is asserted through its own failure contract, which is guarded
// by a GetConsoleMode probe so the same cases read correctly on a real
// terminal.
//
// The _cgets protocol is driven through a seeded line_stash rather than
// a real console: take_line prefers the stash over the device, so
// filling it is exactly what a ReadConsoleW would have done.

#include <catch_amalgamated.hpp>

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <initializer_list>
#include <string>

#include "conio_channel.h"

extern "C" int __cdecl _set_printf_count_output(int);
extern "C" int __cdecl _get_printf_count_output(void);

namespace
{
  namespace conio = mingw_thunk::i::conio;
  namespace shell = mingw_thunk::i::shell;

  bool have_console(DWORD which)
  {
    DWORD mode = 0;
    return GetConsoleMode(GetStdHandle(which), &mode) != 0;
  }

  struct rendered
  {
    bool ok = false;
    int err = 0;
    std::string text;
    size_t units = 0;

    // The console text as UTF-8: the engine's bytes widened for
    // WriteConsoleW, narrowed again so the cases can compare literals.
    void fill(const conio::render_out &out)
    {
      units = out.units;
      text.resize(3 * out.units + 1);
      text.resize(shell::u16_to_u8(out.buf, out.units, &text[0]));
    }
  };

  // va_start/va_end have to live in one frame and the arguments have to
  // arrive through a real C variadic call (a variadic *template* would
  // not set %al, which i686 requires before a double can be read out of
  // the varargs), so the two drivers are plain C variadic functions
  // that fill a rendered in place.
  void narrow_into(rendered *r, uint64_t options, const char *format, ...)
  {
    va_list ap;
    va_start(ap, format);
    conio::render_out out;
    r->ok = conio::render_narrow(options, format, ap, out, &r->err);
    va_end(ap);
    if (r->ok)
      r->fill(out);
  }

  void wide_into(rendered *r, uint64_t options, const wchar_t *format, ...)
  {
    va_list ap;
    va_start(ap, format);
    conio::render_out out;
    r->ok = conio::render_wide(options, format, ap, out, &r->err);
    va_end(ap);
    if (r->ok)
      r->fill(out);
  }

  // The device half needs the holder alive inside the variadic frame, so
  // it is driven from the same C variadic driver: a render that failed
  // never reaches the device, and emit reports that as -1 too.
  int narrow_and_emit(rendered *r, uint64_t options, const char *format, ...)
  {
    va_list ap;
    va_start(ap, format);
    conio::render_out out;
    r->ok = conio::render_narrow(options, format, ap, out, &r->err);
    va_end(ap);
    if (r->ok)
      r->fill(out);
    return conio::emit(out);
  }

#define RENDER_NARROW(r, options, ...) narrow_into(&(r), (options), __VA_ARGS__)
#define RENDER_WIDE(r, options, ...) wide_into(&(r), (options), __VA_ARGS__)

  // Fills the thread's _cgets stash with a synthetic console line, the
  // way a ReadConsoleW would have.
  void seed_line(std::initializer_list<wchar_t> text)
  {
    conio::line_stash &stash = conio::cgets_stash();
    stash.n = 0;
    for (wchar_t c : text) {
      if (stash.n < 128)
        stash.buf[stash.n++] = c;
    }
    stash.pos = 0;
  }
} // namespace

// ------------------------- render: narrow path -------------------------

TEST_CASE("conio narrow render")
{
  rendered r;
  RENDER_NARROW(r, 0, "%d/%s/%.2f", 42, "abc", 1.5);
  REQUIRE(r.ok);
  REQUIRE(r.err == 0);
  REQUIRE(r.text == "42/abc/1.50");
  REQUIRE(r.units == r.text.size());
}

TEST_CASE("conio narrow render widens a non-BMP code point")
{
  // U+1F600 is a surrogate pair in UTF-16, so the console write is two
  // units where the engine produced four UTF-8 bytes
  rendered r;
  RENDER_NARROW(r, 0, "%s", "\xF0\x9F\x98\x80");
  REQUIRE(r.ok);
  REQUIRE(r.units == 2);
  REQUIRE(r.text == "\xF0\x9F\x98\x80");
}

TEST_CASE("conio narrow render is byte-identical to the stdio shell")
{
  // the console render must not introduce a second engine dialect
  rendered r;
  RENDER_NARROW(r, 0, "%s|%5d|%-5s|%+.3e|%#x", "x", 7, "y", 1234.5, 0x2au);
  REQUIRE(r.ok);
  REQUIRE(r.text == "x|    7|y    |+1.234e+03|0x2a");
}

TEST_CASE("conio narrow render: option 0x10 raises the exponent digits")
{
  rendered two;
  RENDER_NARROW(two, 0, "%e", 1.0);
  rendered three;
  RENDER_NARROW(three, 0x10, "%e", 1.0);
  REQUIRE(two.ok);
  REQUIRE(three.ok);
  REQUIRE(two.text == "1.000000e+00");
  REQUIRE(three.text == "1.000000e+000");

  // the guard restores the engine default afterwards
  rendered again;
  RENDER_NARROW(again, 0, "%e", 1.0);
  REQUIRE(again.text == two.text);
}

TEST_CASE("conio narrow render: positional arguments resolve natively")
{
  rendered r;
  RENDER_NARROW(r, 0, "%2$s-%1$s", "one", "two");
  REQUIRE(r.ok);
  REQUIRE(r.text == "two-one");
}

TEST_CASE("conio narrow render gates")
{
  {
    rendered r;
    RENDER_NARROW(r, 0, nullptr, 1);
    REQUIRE_FALSE(r.ok);
    REQUIRE(r.err == EINVAL);
  }
  {
    // %n renders unconditionally (musl semantics): the count lands
    // and the count-output switch reports always-on
    int where = 0;
    rendered r;
    RENDER_NARROW(r, 0, "%d%n", 1, &where);
    REQUIRE(r.ok);
    REQUIRE(r.text == "1");
    REQUIRE(where == 1);
    REQUIRE(_get_printf_count_output() == 1);
    REQUIRE(_set_printf_count_output(0) == 1);
  }
  {
    // %% is not the %n conversion
    rendered r;
    RENDER_NARROW(r, 0, "%%n", 1);
    REQUIRE(r.ok);
    REQUIRE(r.text == "%n");
  }
}

// -------------------------- render: wide path --------------------------

TEST_CASE("conio wide render, ISO conventions")
{
  // without 0x4 the format goes to the engine verbatim, so %s takes a
  // narrow argument
  rendered r;
  RENDER_WIDE(r, 0, L"%s", "abc");
  REQUIRE(r.ok);
  REQUIRE(r.text == "abc");
}

TEST_CASE("conio wide render, legacy wide conventions (0x4)")
{
  // with 0x4 the pre-translator marks %s wide, so the same directive
  // takes a wchar_t*
  rendered r;
  RENDER_WIDE(r, 0x4, L"%s", L"abc");
  REQUIRE(r.ok);
  REQUIRE(r.text == "abc");

  // and %S is the narrow spelling, so it still takes a char*
  const char *narrow = "abc";
  rendered n;
  RENDER_WIDE(n, 0x4, L"%S", narrow);
  REQUIRE(n.ok);
  REQUIRE(n.text == "abc");
}

TEST_CASE("conio wide render: both sides agree on the text")
{
  // a wide format with a wide argument and the equivalent narrow format
  // with a UTF-8 argument produce the same console text
  rendered w;
  RENDER_WIDE(w, 0x4, L"[%s] %d", L"\u5929", 3);
  rendered n;
  RENDER_NARROW(n, 0, "[%s] %d", "\xE5\xA4\xA9", 3);
  REQUIRE(w.ok);
  REQUIRE(n.ok);
  REQUIRE(w.text == n.text);
  REQUIRE(w.text == "[\xE5\xA4\xA9] 3");
}

TEST_CASE("conio wide render: option 0x10 combines with 0x4")
{
  rendered r;
  RENDER_WIDE(r, 0x4 | 0x10, L"%e", 1.0);
  REQUIRE(r.ok);
  REQUIRE(r.text == "1.000000e+000");
}

TEST_CASE("conio wide render gates")
{
  rendered r;
  RENDER_WIDE(r, 0, nullptr, 1);
  REQUIRE_FALSE(r.ok);
  REQUIRE(r.err == EINVAL);
}

// ---------------------------- device contract --------------------------

TEST_CASE("conio device: a rendered payload is one console write")
{
  rendered r;
  // the whole path in one call: render, widen, one console write
  int answer = narrow_and_emit(&r, 0, "hello", 1);
  REQUIRE(r.ok);
  REQUIRE(r.units == 5);
  REQUIRE(answer == (have_console(STD_OUTPUT_HANDLE) ? 5 : -1));
}

TEST_CASE("conio device: a failed render never reaches the device")
{
  // the null-format gate is the render failure within reach headless;
  // %n no longer fails (musl semantics, always enabled)
  rendered r;
  REQUIRE(narrow_and_emit(&r, 0, nullptr, 1) == -1);
  REQUIRE_FALSE(r.ok);
  REQUIRE(r.err == EINVAL);
}

TEST_CASE("conio device: console reads fail headless")
{
  wchar_t line[16];
  size_t units = 0;
  if (have_console(STD_INPUT_HANDLE)) {
    // a real terminal would block here, so only the capacity gate is
    // checked
    REQUIRE(conio::read_cooked_line(line, 1, &units) == false);
  } else {
    REQUIRE_FALSE(conio::read_cooked_line(line, 16, &units));
  }
  // the capacity gate is a pure precondition either way
  REQUIRE_FALSE(conio::read_cooked_line(line, 1, &units));
}

// ------------------------------ _putch ---------------------------------

TEST_CASE("putch: the sequence-length table follows RFC 3629")
{
  REQUIRE(conio::u8_seq_need(0x00) == 1);
  REQUIRE(conio::u8_seq_need(0x7F) == 1);
  REQUIRE(conio::u8_seq_need(0xC2) == 2);
  REQUIRE(conio::u8_seq_need(0xDF) == 2);
  REQUIRE(conio::u8_seq_need(0xE0) == 3);
  REQUIRE(conio::u8_seq_need(0xEF) == 3);
  REQUIRE(conio::u8_seq_need(0xF0) == 4);
  REQUIRE(conio::u8_seq_need(0xF4) == 4);

  // 0x80..0xC1 and 0xF5..0xFF cannot start a sequence
  REQUIRE(conio::u8_seq_need(0x80) == 0);
  REQUIRE(conio::u8_seq_need(0xBF) == 0);
  REQUIRE(conio::u8_seq_need(0xC0) == 0);
  REQUIRE(conio::u8_seq_need(0xC1) == 0);
  REQUIRE(conio::u8_seq_need(0xF5) == 0);
  REQUIRE(conio::u8_seq_need(0xFF) == 0);
}

TEST_CASE("putch: the trail-byte test")
{
  REQUIRE(conio::u8_is_trail(0x80));
  REQUIRE(conio::u8_is_trail(0xBF));
  REQUIRE_FALSE(conio::u8_is_trail(0x7F));
  REQUIRE_FALSE(conio::u8_is_trail(0xC2));
}

TEST_CASE("putch: the decoder rejects what RFC 3629 excludes")
{
  const unsigned char ascii[] = {0x41};
  const unsigned char two[] = {0xC2, 0xA9};
  const unsigned char three[] = {0xE4, 0xB8, 0x96};
  const unsigned char four[] = {0xF0, 0x9F, 0x98, 0x80};

  REQUIRE(conio::putch_decode(ascii, 1) == 0x41);
  REQUIRE(conio::putch_decode(two, 2) == 0xA9);
  REQUIRE(conio::putch_decode(three, 3) == 0x4E16);
  REQUIRE(conio::putch_decode(four, 4) == 0x1F600);

  // overlong two-byte form of '/'
  const unsigned char overlong2[] = {0xC0, 0xAF};
  REQUIRE(conio::putch_decode(overlong2, 2) == 0xFFFD);

  // overlong three-byte form of NUL
  const unsigned char overlong3[] = {0xE0, 0x80, 0x80};
  REQUIRE(conio::putch_decode(overlong3, 3) == 0xFFFD);

  // a UTF-16 surrogate half
  const unsigned char surrogate[] = {0xED, 0xA0, 0x80};
  REQUIRE(conio::putch_decode(surrogate, 3) == 0xFFFD);

  // past U+10FFFF
  const unsigned char toobig[] = {0xF4, 0x90, 0x80, 0x80};
  REQUIRE(conio::putch_decode(toobig, 4) == 0xFFFD);
}

TEST_CASE("putch: a leading byte parks and returns the character")
{
  bool console = have_console(STD_OUTPUT_HANDLE);
  conio::putch_park() = {};
  REQUIRE(conio::putch(0xE4) == 0xE4); // parked, no device write
  REQUIRE(conio::putch_park().used == 1);
  REQUIRE(conio::putch(0xB8) == 0xB8); // still incomplete
  REQUIRE(conio::putch_park().used == 2);
  // the completing byte writes the code point, so it answers with the
  // character on a console and with EOF on a redirect
  REQUIRE(conio::putch(0x96) == (console ? 0x96 : EOF));
  REQUIRE(conio::putch_park().used == 0);
}

TEST_CASE("putch: a byte that cannot continue flushes the park")
{
  bool console = have_console(STD_OUTPUT_HANDLE);
  conio::putch_park() = {};
  REQUIRE(conio::putch(0xE4) == 0xE4);
  // the park flushes U+FFFD and 'x' is reprocessed as a fresh byte
  REQUIRE(conio::putch('x') == (console ? 'x' : EOF));
  REQUIRE(conio::putch_park().used == 0);
}

TEST_CASE("putch: a stray byte is one replacement")
{
  int answer = have_console(STD_OUTPUT_HANDLE) ? 1 : EOF;
  conio::putch_park() = {};
  REQUIRE(conio::putch(0xFF) == answer);
  REQUIRE(conio::putch(0x80) == answer);
  REQUIRE(conio::putch(0xC0) == answer);
  REQUIRE(conio::putch_park().used == 0);
}

TEST_CASE("putch: an ASCII byte goes straight out")
{
  int answer = have_console(STD_OUTPUT_HANDLE) ? 'A' : EOF;
  conio::putch_park() = {};
  REQUIRE(conio::putch('A') == answer); // no park at all
  REQUIRE(conio::putch_park().used == 0);
}

// ------------------------------ _cgets ---------------------------------

TEST_CASE("cgets: the protocol converts a console line to UTF-8")
{
  seed_line({L'a', 0x4E16, L'b', L'c'});
  char buf[16];
  size_t got = 0;
  memset(buf, 'X', sizeof buf);

  REQUIRE(conio::cgets_s_body(buf, sizeof buf, &got) == 0);
  REQUIRE(got == 6); // a(1) + U+4E16(3) + b(1) + c(1)
  REQUIRE(strcmp(buf, "a\xE4\xB8\x96""bc") == 0);
  REQUIRE(buf[got] == '\0');
}

TEST_CASE("cgets: a code point is never split by the buffer end")
{
  // 2 payload units, 2 bytes of room: the whole 3-byte character is
  // dropped rather than written half, and it comes back on the next call
  seed_line({0x4E16, L'a'});
  char small[3]; // room for 2 bytes
  size_t got = 0;

  REQUIRE(conio::cgets_s_body(small, sizeof small, &got) == 0);
  REQUIRE(got == 0);
  REQUIRE(small[0] == '\0');

  // the stash still holds the code point, so a roomier buffer sees it
  char roomy[16];
  REQUIRE(conio::cgets_s_body(roomy, sizeof roomy, &got) == 0);
  REQUIRE(got == 4);
  REQUIRE(strcmp(roomy, "\xE4\xB8\x96""a") == 0);
}

TEST_CASE("cgets: the buffer is cleared before anything is written")
{
  seed_line({L'z'});
  char buf[8];
  memset(buf, 'X', sizeof buf);
  size_t got = 1;

  REQUIRE(conio::cgets_s_body(buf, sizeof buf, &got) == 0);
  REQUIRE(got == 1);
  REQUIRE(buf[0] == 'z');
  REQUIRE(buf[1] == '\0');
  // _RESET_STRING reached the whole buffer
  for (size_t i = 2; i < sizeof buf; i++)
    REQUIRE(buf[i] == '\0');
}

TEST_CASE("cgets: argument gates")
{
  char buf[8];
  size_t got = 0;
  errno = 0;
  REQUIRE(conio::cgets_s_body(nullptr, 8, &got) == EINVAL);
  REQUIRE(errno == EINVAL);

  errno = 0;
  REQUIRE(conio::cgets_s_body(buf, 0, &got) == EINVAL);
  REQUIRE(errno == EINVAL);

  errno = 0;
  REQUIRE(conio::cgets_s_body(buf, 8, nullptr) == EINVAL);
  REQUIRE(errno == EINVAL);
}

TEST_CASE("cgets: an exhausted stash with no console is EBADF")
{
  conio::line_stash &stash = conio::cgets_stash();
  stash.n = stash.pos = 0;

  char buf[8];
  size_t got = 99;
  errno = 0;
  if (have_console(STD_INPUT_HANDLE)) {
    // a real terminal would block here, so only the gate ordering is
    // checked
    REQUIRE(conio::cgets_s_body(buf, 0, &got) == EINVAL);
    return;
  }
  REQUIRE(conio::cgets_s_body(buf, sizeof buf, &got) == EBADF);
  REQUIRE(errno == EBADF);
  REQUIRE(got == 0);
}

TEST_CASE("cgets: a surrogate pair stays one code point")
{
  seed_line({0xD83D, 0xDE00}); // U+1F600
  char buf[16];
  size_t got = 0;

  REQUIRE(conio::cgets_s_body(buf, sizeof buf, &got) == 0);
  REQUIRE(got == 4);
  REQUIRE(strcmp(buf, "\xF0\x9F\x98\x80") == 0);
}

TEST_CASE("cgets: a lone surrogate becomes the replacement")
{
  seed_line({0xD83D, L'x'});
  char buf[16];
  size_t got = 0;

  REQUIRE(conio::cgets_s_body(buf, sizeof buf, &got) == 0);
  REQUIRE(got == 4); // U+FFFD is three bytes
  REQUIRE(strcmp(buf, "\xEF\xBF\xBD""x") == 0);
}

TEST_CASE("cgets: the stash keeps the order across calls")
{
  seed_line({L'a', L'b', L'c', L'd', L'e'});

  char two[3];
  size_t got = 0;
  REQUIRE(conio::cgets_s_body(two, sizeof two, &got) == 0);
  REQUIRE(strcmp(two, "ab") == 0);

  char one[2];
  REQUIRE(conio::cgets_s_body(one, sizeof one, &got) == 0);
  REQUIRE(strcmp(one, "c") == 0);

  char rest[16];
  REQUIRE(conio::cgets_s_body(rest, sizeof rest, &got) == 0);
  REQUIRE(strcmp(rest, "de") == 0);
}

TEST_CASE("cgets: code point length table")
{
  REQUIRE(conio::cp_u8_len(0x7F) == 1);
  REQUIRE(conio::cp_u8_len(0x80) == 2);
  REQUIRE(conio::cp_u8_len(0x7FF) == 2);
  REQUIRE(conio::cp_u8_len(0x800) == 3);
  REQUIRE(conio::cp_u8_len(0xFFFF) == 3);
  REQUIRE(conio::cp_u8_len(0x10000) == 4);
  REQUIRE(conio::cp_u8_len(0x10FFFF) == 4);
}
