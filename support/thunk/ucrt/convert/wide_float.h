#pragma once

// M11 wide float-parse view (plan-3 §3.3, D4 view): transcode the wide
// string to UTF-8 (lone surrogates -> U+FFFD), run the narrow musl engine,
// then fold the consumed byte count back to a wide index.
//
// endptr theorem: every byte the engine can consume is ASCII (< 0x80) —
// digits, '.', 'e'/'E', '+'/'-', 'x'/'X', inf/nan letters, '(', ')', '_'
// and the nan payload alnum set.  UTF-8 encodings of non-ASCII code points
// never enter the consumed prefix, so consumed bytes == consumed wide
// units and *end = ws + consumed is exact (no per-code-point walk needed).
//
// Documented divergences vs native ucrtbase (wine anchors, plan-3 §3.2/§3.7):
//  * native wcstod skips iswspace leading whitespace (U+3000, U+00A0);
//    the musl engine skips the ASCII space set only.
//  * native wcstod(L"nan(0x10)") stops after L"nan" (endCh == L'(');
//    the narrow engine consumes the payload — we keep the narrow shape.

#include <thunk/_common.h>
#include <thunk/buffer.h>
#include <thunk/unicode.h>
#include <thunk/u8crt/musl.h>

#include <errno.h>
#include <stdint.h>
#include <wchar.h>

namespace mingw_thunk
{
  namespace u8crt_convert
  {
    // ws -> UTF-8; lone surrogates become U+FFFD (protocol-neutral: those
    // bytes are never consumed by the ASCII-only engine).  Returns false
    // only on allocation failure.
    inline bool
    wide_to_utf8(const wchar_t *ws, d::buffer<256> &out) noexcept
    {
      out.clear();
      for (const wchar_t *p = ws; *p; ++p) {
        char32_t c = *p;
        if (i::u16_is_high(*p)) {
          if (p[1] && i::u16_is_low(p[1])) {
            c = i::u16_dec_2({*p, p[1]});
            ++p;
          } else {
            c = 0xfffd;
          }
        } else if (i::u16_is_low(*p)) {
          c = 0xfffd;
        }

        if (c < 0x80) {
          if (!out.push_back(char(c)))
            return false;
        } else if (c < 0x800) {
          const d::u8_2 e = i::u8_enc_2(c);
          if (!out.append((const char *)&e, 2))
            return false;
        } else if (c < 0x10000) {
          const d::u8_3 e = i::u8_enc_3(c);
          if (!out.append((const char *)&e, 3))
            return false;
        } else {
          const d::u8_4 e = i::u8_enc_4(c);
          if (!out.append((const char *)&e, 4))
            return false;
        }
      }
      return true;
    }

    template <typename T, T (*Engine)(const char *, char **)>
    static T
    wide_float_parse(const wchar_t *ws, wchar_t **end) noexcept
    {
      if (end)
        *end = const_cast<wchar_t *>(ws);
      if (!ws)
        return 0; // caller handles errno

      d::buffer<256> u8;
      if (!wide_to_utf8(ws, u8)) {
        errno = ENOMEM;
        return 0;
      }

      char *p = nullptr;
      T value = Engine(u8.data(), &p);
      const size_t consumed = size_t(p - u8.data());
      if (end)
        *end = const_cast<wchar_t *>(ws) + consumed;
      return value;
    }
  } // namespace u8crt_convert
} // namespace mingw_thunk
