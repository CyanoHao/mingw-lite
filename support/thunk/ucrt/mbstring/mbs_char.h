#pragma once

// plan-3 §5.2 M13c: the single-character half of the UTF-8 multibyte
// engine -- the g4 faces (fold / kana / Shift-JIS / one-character copy /
// length) and the two pseudo-tables.  It is expressed in terms of the
// M13a byte-role primitives (include/thunk/u8crt/utf8_mbs.h) and reuses
// the M13b string engine's secure-crt helpers (ucrt/mbstring/mbs_str.h),
// because `_mbccpy_s` has the same `_s` protocol as `_mbscpy_s`: EINVAL
// for a null/empty destination, EINVAL + `*dst = '\0'` for a null
// source, ERANGE for a destination too small, EILSEQ for a source that
// ends in half a character.
//
// The decisions here are D7a..D7d of plan-3 §5.0.3 (revised), and the
// two that matter most are that the reference's Shift-JIS *arithmetic*
// is not ported (it is arithmetic on the wrong alphabet under UTF-8) and
// that the four Shift-JIS-only faces are identity, because this layer's
// codepage is never 932 -- which is exactly the branch the reference
// itself takes before its tables are ever consulted.

#include "mbs_str.h"

namespace mingw_thunk
{
  namespace mbstring
  {
    // The widest UTF-8 character.  The reference's non-secure `_mbccpy`
    // hands its secure body a fixed 2 -- "enough for any character in
    // this codepage" -- and 4 is that quantity here.
    constexpr size_t kMaxWidth = 4;

    // ----------------------------------------------------------------
    // case fold (D7a)
    // ----------------------------------------------------------------

    // A value that names a character at all: any scalar value.  The
    // surrogates and the range above U+10FFFF are the UTF-8 encoder's
    // own two refusals, and they are the same two this layer refuses
    // everywhere else, so a fold of one is "nothing to do" rather than
    // an error: the input comes back and errno is not touched.
    inline bool scalar_ok(unsigned int c) noexcept
    {
      return c <= 0x10FFFFu && (c < 0xD800u || c > 0xDFFFu);
    }

    // In = codepoint, out = codepoint.  The reference's argument is a
    // DBCS code packed as `lead << 8 | trail` and its body unpacks,
    // folds two bytes and repacks; a UTF-8 sequence does not fit that
    // slot, so the packed form is dropped and the whole scalar travels
    // in the argument instead (§5.0.3 D7a).  The fold itself is the
    // string family's -- ASCII through the byte table, everything else
    // through the codepoint folder -- and the only thing added here is
    // the scalar guard, because a value that is not a character is the
    // one case the `unsigned int` argument can express and a decoded
    // BMP scalar cannot.
    inline unsigned int char_lower(unsigned int c) noexcept
    {
      if (!scalar_ok(c))
        return c;
      return static_cast<unsigned int>(fold_lower(static_cast<char32_t>(c)));
    }

    inline unsigned int char_upper(unsigned int c) noexcept
    {
      if (!scalar_ok(c))
        return c;
      return static_cast<unsigned int>(fold_upper(static_cast<char32_t>(c)));
    }

    // ----------------------------------------------------------------
    // kana shift (D7c)
    // ----------------------------------------------------------------

    // The reference's `c -= 0xa1` / `c += 0xa1` is Shift-JIS byte
    // arithmetic; run on a codepoint it would answer U+3001 for U+30A2,
    // which is not hiragana.  The two kana blocks are the same length
    // and line up exactly 0x60 apart, so the real mapping is one
    // addition in one direction and one subtraction in the other, and
    // every character outside the two ranges -- including the long vowel
    // mark U+30FC, which sits between the ranges rather than in either
    // -- is left alone.
    inline unsigned int to_hira(unsigned int c) noexcept
    {
      if (c >= 0x30A1u && c <= 0x30F6u)
        return c - 0x60u;
      return c;
    }

    inline unsigned int to_kata(unsigned int c) noexcept
    {
      if (c >= 0x3041u && c <= 0x3096u)
        return c + 0x60u;
      return c;
    }

    // ----------------------------------------------------------------
    // Shift-JIS only (D7b)
    // ----------------------------------------------------------------

    // All four faces open with `if (mbcodepage != _KANJI_CP) return c;`,
    // so under any other codepage their tables and their `errno = EILSEQ`
    // for out-of-range input are unreachable and the answer is the input
    // unchanged.  This layer's codepage is always UTF-8, so that early
    // return is the whole of all four faces -- and producing Shift-JIS
    // bytes inside a UTF-8 product would be encoding corruption, so the
    // identity is chosen rather than merely inherited (R-13).
    inline unsigned int bbtombc(unsigned int c) noexcept
    {
      return c;
    }

    inline unsigned int ctombb(unsigned int c) noexcept
    {
      return c;
    }

    inline unsigned int jistojms(unsigned int c) noexcept
    {
      return c;
    }

    inline unsigned int jmstojis(unsigned int c) noexcept
    {
      return c;
    }

    // ----------------------------------------------------------------
    // one-character length (D8d)
    // ----------------------------------------------------------------

    // The byte length of the whole character at p.  A byte that does not
    // begin a complete well-formed sequence -- a truncated lead, a stray
    // trail byte, an overlong form -- answers 1, which is the reference's
    // conservative guard around its own lead-byte test widened from one
    // trail byte to every UTF-8 width.  A null pointer has no character
    // to measure: 0, and EINVAL (the reference dereferences and the
    // reference shell has no null check at all).
    inline size_t clen(const unsigned char *p) noexcept
    {
      if (!p) {
        _set_errno(EINVAL);
        return 0;
      }
      const unsigned w = seq_ok(p);
      return w ? w : 1;
    }

    // ----------------------------------------------------------------
    // one-character copy
    // ----------------------------------------------------------------

    // Copy exactly one character, whole.  *copied -- when the caller
    // wants it -- is the number of bytes written, and the destination
    // gets the character alone: no terminator is appended, which is what
    // the reference's `*_Dst++ = *_Src++; *_Dst = *_Src;` does too.
    inline errno_t ccpy_s(unsigned char *dst,
                          size_t size,
                          int *copied,
                          const unsigned char *src) noexcept
    {
      if (copied)
        *copied = 0;
      if (!valid_string(dst, size))
        return EINVAL;
      if (!src) {
        *dst = '\0';
        _set_errno(EINVAL);
        return EINVAL;
      }
      if (tail_cut(src)) {
        // A lead byte whose character never finishes is the reference's
        // "lead byte followed by the terminator": copy only the
        // terminator, say one byte was copied, and report EILSEQ.
        *dst = '\0';
        if (copied)
          *copied = 1;
        _set_errno(EILSEQ);
        return EILSEQ;
      }

      const unsigned w = step(src);
      if (size < w) {
        *dst = '\0';
        _set_errno(ERANGE);
        return ERANGE;
      }
      memcpy(dst, src, w);
      if (copied)
        *copied = static_cast<int>(w);
      return 0;
    }

    // The non-secure form.  The reference spells it as its own secure
    // body called with a size of 2; the encoding's widest character is
    // what that number means, and here that is four.
    inline void ccpy(unsigned char *dst, const unsigned char *src) noexcept
    {
      ccpy_s(dst, kMaxWidth, nullptr, src);
    }

    // ----------------------------------------------------------------
    // the two pseudo-tables (D7d)
    // ----------------------------------------------------------------

    // `__p__mbctype` is a table of *bit masks*, not of `_MBC_*` return
    // values: `_MBC_LEAD`/`_MBC_TRAIL`/`_MBC_SINGLE`/`_MBC_ILLEGAL` are
    // what the `_mbbtype`/`_mbsbtype` family returns, while the table
    // holds `_SBUP`/`_SBLOW`/`_M1`/`_M2`.  The bytes this layer can name
    // are its own: ASCII letters get the upper/lower bits the reference's
    // `setSBUpLow` gives them, the UTF-8 lead and trail ranges get `_M1`
    // and `_M2`, and the two bytes that can begin no sequence (C0, C1 --
    // overlong leads) and the range above F4 get nothing, which is what
    // makes a reader of the table call them `_MBC_ILLEGAL`.
    //
    // Index 0 is the reference's EOF slot and is zero like the rest of
    // the unused range; the table is 257 entries because that is its
    // documented extent, not because anything indexes past 255.
    inline const unsigned char *mbctype_table() noexcept
    {
      static const unsigned char table[257] = {
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10,
          0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
          0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
          0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x20,
          0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
          0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
          0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x08, 0x08, 0x08,
          0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
          0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
          0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
          0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
          0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
          0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00, 0x04, 0x04, 0x04, 0x04,
          0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
          0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
          0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
          0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
          0x04, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00,
      };
      return table;
    }
  } // namespace mbstring
} // namespace mingw_thunk
