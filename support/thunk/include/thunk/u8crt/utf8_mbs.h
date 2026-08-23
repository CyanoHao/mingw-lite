#pragma once

#include <stddef.h>

#include <wchar.h>
#include <wctype.h>
#include <windows.h>

#include "c_utf8_tables.h"
#include "mbstring.h"

// plan-3 §5.1: the UTF-8 multibyte engine shared by every ucrt/mbstring
// face.  The whole family is defined against UTF-8 byte roles and
// code-point semantics (D7/D8 in plan-3 §5.0.3); the Shift-JIS shapes in
// reference/ucrt/mbstring/*.cpp are the structural source, not the
// numeric one.
namespace mingw_thunk
{
  namespace __crt_mbs
  {
    // ----------------------------------------------------------------
    // byte roles (RFC 3629)
    // ----------------------------------------------------------------

    // 0xC0/0xC1 (overlong two-byte leads) and 0xF5..0xFF (> U+10FFFF)
    // are not leads.  Below 0x80 the "lead" of a one-byte character is
    // the character itself, which is why seq_width() has a fast path
    // below and the predicate does not.
    inline bool lead_byte(unsigned char b) noexcept
    {
      return b >= 0xC2 && b <= 0xF4;
    }

    inline bool trail_byte(unsigned char b) noexcept
    {
      return (b & 0xC0) == 0x80;
    }

    // Byte count implied by a lead byte; 0 when b cannot start a
    // sequence.  A byte < 0x80 (including NUL) is a one-byte character.
    inline unsigned seq_width(unsigned char b) noexcept
    {
      if (b < 0x80)
        return 1;
      if (b < 0xC2)
        return 0;
      if (b < 0xE0)
        return 2;
      if (b < 0xF0)
        return 3;
      if (b < 0xF5)
        return 4;
      return 0;
    }

    enum : int
    {
      DEC_OK = 0,   // *pcp / *plen describe one scalar value
      DEC_BAD = -1, // not a well-formed sequence here
      DEC_TRUNC = -2 // the NUL terminator landed inside the sequence
    };

    // Decodes the character at s (a NUL-terminated buffer).  *plen is
    // always 1: every failure path reports "skip one byte" so the
    // scanning families keep making progress the way the reference shells
    // do once their _ASSERTE is compiled out.  A lone continuation byte
    // (0x80..0xBF) and the non-lead bytes 0xC0/0xC1/0xF5..0xFF are
    // DEC_BAD, not one-byte characters.  NUL decodes to scalar 0 with
    // width 1; callers test *pcp for the end of string.
    inline int decode(const char *s, char32_t *pcp, unsigned *plen) noexcept
    {
      const unsigned char *b = reinterpret_cast<const unsigned char *>(s);
      const unsigned w = seq_width(b[0]);
      *plen = 1;
      if (w == 0)
        return DEC_BAD; // 0x80..0xBF standalone, or 0xC0/0xC1/0xF5+
      if (w == 1) {
        *pcp = b[0];
        return DEC_OK;
      }

      char32_t cp;
      switch (w) {
      case 2:
        cp = (char32_t)(b[0] & 0x1F);
        break;
      case 3:
        cp = (char32_t)(b[0] & 0x0F);
        break;
      default:
        cp = (char32_t)(b[0] & 0x07);
        break;
      }

      for (unsigned i = 1; i < w; ++i) {
        if (b[i] == '\0')
          return DEC_TRUNC;
        if (!trail_byte(b[i]))
          return DEC_BAD;
        cp = (cp << 6) | (char32_t)(b[i] & 0x3F);
      }

      // the four second-byte ranges that carry RFC 3629's exclusions
      if (w == 2 && cp < 0x80)
        return DEC_BAD;
      if (w == 3 && b[0] == 0xE0 && b[1] < 0xA0)
        return DEC_BAD;
      if (w == 3 && b[0] == 0xED && b[1] > 0x9F)
        return DEC_BAD;
      if (w == 4 && b[0] == 0xF0 && b[1] < 0x90)
        return DEC_BAD;
      if (w == 4 && b[0] == 0xF4 && b[1] > 0x8F)
        return DEC_BAD;

      *pcp = cp;
      *plen = w;
      return DEC_OK;
    }

    // Character width at s: the decoded byte count for a well-formed
    // sequence, 1 for anything else (including a truncated tail).  This
    // is _mbclen's contract (plan-3 D8d: the reference's conservative
    // `c[1] != '\0'` guard, widened to RFC 3629).
    inline unsigned char_width(const char *s) noexcept
    {
      char32_t cp;
      unsigned w;
      if (decode(s, &cp, &w) != DEC_OK)
        return 1;
      return (unsigned char)w;
    }

    // Byte offset of the first byte of the character ending at p, or 0
    // when p is not inside a sequence at all.  Bounded by the four-byte
    // maximum, so the mbsdec family never scans further than this.
    inline size_t back_off(const unsigned char *s,
                           const unsigned char *p) noexcept
    {
      if (p <= s || !trail_byte(*p))
        return 0;
      for (size_t back = 1; back <= 3 && (size_t)(p - s) >= back + 1; ++back)
        if (lead_byte(p[-static_cast<ptrdiff_t>(back)]))
          return back;
      return 0;
    }

    // ----------------------------------------------------------------
    // code-point classification (plan-3 D8h)
    // ----------------------------------------------------------------

    // The CT_CTYPE1 bits the _ismbc* family reads.  0x0100 is the
    // caseless-alpha bit that mingw's `_ALPHA` macro (0x0100|_UPPER|
    // _LOWER) exposes: GetStringTypeW sets it for letters with no case
    // (CJK, kana) and clears the two case bits, so the alpha family has
    // to consult it or `_ismbcalpha(0x4E00)` would answer false.
    constexpr unsigned short T_UPPER = 0x0001;
    constexpr unsigned short T_LOWER = 0x0002;
    constexpr unsigned short T_DIGIT = 0x0004;
    constexpr unsigned short T_SPACE = 0x0008;
    constexpr unsigned short T_PUNCT = 0x0010;
    constexpr unsigned short T_CONTROL = 0x0020;
    constexpr unsigned short T_BLANK = 0x0040;
    constexpr unsigned short T_HEX = 0x0080;
    constexpr unsigned short T_CASELESS_ALPHA = 0x0100;

    inline bool scalar_valid(unsigned int c) noexcept
    {
      return c <= 0x10FFFF && !(c >= 0xD800 && c <= 0xDFFF);
    }

    // CT_CTYPE1 word for a scalar value.  Below U+0100 the two static
    // tables answer (wine-anchored copies of the native initial C
    // tables, M8/M10, with the '\t' _BLANK correction); above it the
    // OS classification is asked for.  The OS table decides the
    // Latin-1-up boundary, so its opinion is the product's opinion
    // there (documented divergence from the narrow table).
    inline unsigned short ctype_word(unsigned int c) noexcept
    {
      if (c < 0x80)
        return i::u8_pctype[c];
      if (c < 0x100)
        return i::u8_pwctype[c];
      if (c > 0xFFFF || (c >= 0xD800 && c <= 0xDFFF))
        return 0; // not a scalar value; the wchar_t slot would alias

      wchar_t src = (wchar_t)c;
      WORD t = 0;
      if (GetStringTypeW(CT_CTYPE1, &src, 1, &t))
        return t;
      return 0;
    }

    inline bool is_alpha_cp(unsigned int c) noexcept
    {
      if (!scalar_valid(c))
        return false;
      const unsigned short t = ctype_word(c);
      return (t & (T_UPPER | T_LOWER | T_CASELESS_ALPHA)) != 0;
    }

    inline bool is_alnum_cp(unsigned int c) noexcept
    {
      if (!scalar_valid(c))
        return false;
      const unsigned short t = ctype_word(c);
      return (t & (T_UPPER | T_LOWER | T_CASELESS_ALPHA | T_DIGIT)) != 0;
    }

    inline bool is_digit_cp(unsigned int c) noexcept
    {
      if (!scalar_valid(c))
        return false;
      return (ctype_word(c) & T_DIGIT) != 0;
    }

    inline bool is_lower_cp(unsigned int c) noexcept
    {
      if (!scalar_valid(c))
        return false;
      return (ctype_word(c) & T_LOWER) != 0;
    }

    inline bool is_upper_cp(unsigned int c) noexcept
    {
      if (!scalar_valid(c))
        return false;
      return (ctype_word(c) & T_UPPER) != 0;
    }

    inline bool is_space_cp(unsigned int c) noexcept
    {
      if (!scalar_valid(c))
        return false;
      return (ctype_word(c) & T_SPACE) != 0;
    }

    inline bool is_blank_cp(unsigned int c) noexcept
    {
      if (!scalar_valid(c))
        return false;
      return (ctype_word(c) & T_BLANK) != 0;
    }

    inline bool is_punct_cp(unsigned int c) noexcept
    {
      if (!scalar_valid(c))
        return false;
      return (ctype_word(c) & T_PUNCT) != 0;
    }

    inline bool is_print_cp(unsigned int c) noexcept
    {
      if (!scalar_valid(c))
        return false;
      return (ctype_word(c) & T_CONTROL) == 0;
    }

    inline bool is_graph_cp(unsigned int c) noexcept
    {
      if (!scalar_valid(c))
        return false;
      return is_print_cp(c) && !is_space_cp(c);
    }

    // ----------------------------------------------------------------
    // block ranges (plan-3 D7c / D8f / D8g)
    // ----------------------------------------------------------------

    inline bool in_range(unsigned int c, unsigned int lo,
                         unsigned int hi) noexcept
    {
      return c >= lo && c <= hi;
    }

    // Hiragana proper (D7c's convertible domain); the voiced/semi-voiced
    // marks at U+3097..U+309F have no katakana counterpart and stay out
    // of both the conversion and the predicate.
    inline bool is_hiragana(unsigned int c) noexcept
    {
      return in_range(c, 0x3041, 0x3096);
    }

    inline bool is_katakana(unsigned int c) noexcept
    {
      return in_range(c, 0x30A1, 0x30F6);
    }

    // JIS X 0208 row 1: the fullwidth forms plus the CJK punctuation
    // block.  Approximation, no native anchor (D8g).
    inline bool is_mb_symbol(unsigned int c) noexcept
    {
      return in_range(c, 0x3000, 0x303F) || in_range(c, 0xFF01, 0xFF60) ||
             in_range(c, 0xFFE0, 0xFFE6);
    }

    // JIS level approximation (D8f): level 0 is everything the CJK
    // planes hold before the ideographs, level 1 the common unified
    // ideographs, level 2 the extension/compatibility blocks.  The
    // three ranges are disjoint and their union is the CJK-plane
    // non-Han part plus the Han blocks.  Not a JIS level oracle.
    inline bool is_mb_level0(unsigned int c) noexcept
    {
      return in_range(c, 0x3000, 0x33FF);
    }

    inline bool is_mb_level1(unsigned int c) noexcept
    {
      return in_range(c, 0x4E00, 0x9FA5);
    }

    inline bool is_mb_level2(unsigned int c) noexcept
    {
      return in_range(c, 0x3400, 0x4DBF) || in_range(c, 0x9FA6, 0x9FFF) ||
             in_range(c, 0xF900, 0xFAFF);
    }

    // ----------------------------------------------------------------
    // _MBC_* return values (mbctype.h; note these are the *results*, the
    // pseudo-table stores the _MS/_MP/_M1/_M2 flag bits instead)
    // ----------------------------------------------------------------

    constexpr int MBC_SINGLE = 0;
    constexpr int MBC_LEAD = 1;
    constexpr int MBC_TRAIL = 2;
    constexpr int MBC_ILLEGAL = -1;

    // Reference mbbtype.cpp state machine with the UTF-8 byte roles
    // swapped in (plan-3 D8a; wine reproduces the machine exactly, only
    // its own lead/trail tables differ).
    inline int byte_type(unsigned char c, int ctype) noexcept
    {
      if (ctype == MBC_LEAD)
        return trail_byte(c) ? MBC_TRAIL : MBC_ILLEGAL;

      if (lead_byte(c))
        return MBC_LEAD;
      if (c < 0x80 && is_print_cp(c))
        return MBC_SINGLE;
      return MBC_ILLEGAL;
    }

    // Walks s -> pos one character at a time.  Returns the address of the
    // character that *contains* pos (its first byte, which is pos itself
    // when pos is a character start), or nullptr when pos is not
    // reachable: a null argument, pos before s, or the terminator in
    // between.  A malformed byte advances one position, as the reference
    // shells do once their _ASSERTE is compiled out.
    inline const unsigned char *walk_to(const unsigned char *s,
                                        const unsigned char *pos) noexcept
    {
      if (!s || !pos || pos < s)
        return nullptr;

      const unsigned char *p = s;
      for (;;) {
        if (p >= pos)
          return p;
        if (*p == '\0')
          return nullptr;
        char32_t cp;
        unsigned w;
        if (decode(reinterpret_cast<const char *>(p), &cp, &w) != DEC_OK)
          w = 1;
        if (p + w > pos)
          return p; // pos lands inside this character
        p += w;
      }
    }

    // Role of the byte at s + off inside the string s, by UTF-8 context
    // (plan-3 D8b).  Distinguishes a sequence's first byte from any of
    // its continuations, which the DBCS lead/trail pair cannot.  off at
    // or past the terminator is _MBC_ILLEGAL, as is a byte that is
    // neither ASCII-print nor part of a well-formed sequence.
    inline int string_byte_type(const unsigned char *s, size_t off) noexcept
    {
      if (!s)
        return MBC_ILLEGAL;

      const unsigned char *pos = s + off;
      const unsigned char *p = walk_to(s, pos);
      if (!p || *p == '\0')
        return MBC_ILLEGAL;
      if (p != pos)
        return MBC_TRAIL; // only reachable for a well-formed wide char

      char32_t cp;
      unsigned w;
      if (decode(reinterpret_cast<const char *>(p), &cp, &w) != DEC_OK)
        return MBC_ILLEGAL;
      if (w == 1)
        return (p[0] < 0x80 && is_print_cp(p[0])) ? MBC_SINGLE : MBC_ILLEGAL;
      return MBC_LEAD;
    }

    // _ismbslead: is `pos` the first byte of a well-formed character?
    // _ismbstrail: is `pos` a continuation byte of one that started
    // earlier?  (plan-3 D8c; the reference declares both meaningless
    // for UTF-8 and returns 0 everywhere.)
    inline bool ptr_is_lead(const unsigned char *s,
                            const unsigned char *pos) noexcept
    {
      const unsigned char *p = walk_to(s, pos);
      if (!p || p != pos || *p == '\0')
        return false;

      char32_t cp;
      unsigned w;
      if (decode(reinterpret_cast<const char *>(p), &cp, &w) != DEC_OK)
        return false;
      return w > 1;
    }

    inline bool ptr_is_trail(const unsigned char *s,
                             const unsigned char *pos) noexcept
    {
      if (!s || !pos || pos <= s || !trail_byte(*pos))
        return false;

      const size_t back = back_off(s, pos);
      if (back == 0)
        return false;

      char32_t cp;
      unsigned w;
      return decode(reinterpret_cast<const char *>(pos - back), &cp, &w) ==
                 DEC_OK &&
             w > back;
    }
  } // namespace __crt_mbs
} // namespace mingw_thunk
