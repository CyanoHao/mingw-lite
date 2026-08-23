#pragma once

// plan-3 §5.2 M13b: the string half of the UTF-8 multibyte engine — the
// 105 `_mbs*` faces (locate / walk / compare / copy+cat / set / case /
// reverse / tokenize).  The byte-role and classification primitives live
// in include/thunk/u8crt/utf8_mbs.h (M13a); everything here is expressed
// in terms of them, so a character means the same thing across both
// halves.  The decision record is plan-3 §5.0.2 (wine anchoring) and the
// D9 series in §5.0.3.
//
// Three rules run through the whole file:
//
//   * the family's own "n" / "nb" split survives verbatim: a count given to
//     a plain `n` face counts **characters**, and a count given to an `nb`
//     face counts **bytes** (D9q..D9t).  The reference shells are the
//     authority and they are not consistent with the spelling in one place
//     -- `_mbsnccnt` takes bytes and returns characters, `_mbsnbcnt` takes
//     characters and returns bytes -- so each face is anchored individually
//     rather than inferred from its name.  The pair is the reason the
//     family exists, so nothing here is allowed to quietly make the two the
//     same function.
//
//   * a byte bound never splits a character.  The DBCS shells can overrun
//     by a byte or leave an orphaned lead; the UTF-8 forms stop before the
//     boundary instead, which is a deliberate hardening (plan-3 §5.0.3,
//     "total, never half a sequence").
//
//   * the "_s" protocol is the M7/M8 secure-crt shape the reference
//     spells out in reference/ucrt/inc/corecrt_internal_securecrt.h:
//     EINVAL for a null/empty pair, EINVAL + buffer reset for a null
//     source, ERANGE for insufficient capacity, EILSEQ for a truncated
//     trailing sequence, and a zero-filled tail on success.

#include <thunk/u8crt/utf8_mbs.h>

#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

namespace mingw_thunk
{
  namespace mbstring
  {
    // The M13a primitives this half builds on, pulled in unqualified so
    // the walking code below reads as walking code and not as namespace
    // bookkeeping.
    using __crt_mbstring::__c32rtomb_utf8;

    using __crt_mbs::back_off;
    using __crt_mbs::DEC_OK;
    using __crt_mbs::decode;
    using __crt_mbs::scalar_valid;
    using __crt_mbs::seq_width;
    using __crt_mbs::trail_byte;
    using __crt_mbs::walk_to;

    // ----------------------------------------------------------------
    // walking primitives
    // ----------------------------------------------------------------

    // Byte width of the well-formed sequence at p, or 0 when p holds a
    // malformed byte or a sequence the terminator cuts short.  Every
    // "how wide is this character" question in the family is answered by
    // this one, so the counting functions cannot disagree about where a
    // character begins.
    inline unsigned seq_ok(const unsigned char *p) noexcept
    {
      char32_t cp;
      unsigned w;
      return decode(reinterpret_cast<const char *>(p), &cp, &w) == DEC_OK ? w
                                                                          : 0;
    }

    // Sequence width with the family's "malformed advances one byte"
    // recovery (the reference shells keep scanning once their _ASSERTE
    // is compiled out).  NUL is one byte, which is what lets every loop
    // below use `*p` as its terminator test.
    inline unsigned step(const unsigned char *p) noexcept
    {
      const unsigned w = seq_ok(p);
      return w ? w : 1;
    }

    // Scalar at p.  A byte that does not start a well-formed sequence
    // stands for itself (D9f): the comparison families have to stay
    // total, and must not report "end of string" for a byte that is not
    // the terminator — the trap the reference's `c = 0` on a naked lead
    // byte sets.
    inline char32_t scalar_at(const unsigned char *p) noexcept
    {
      char32_t cp;
      unsigned w;
      if (decode(reinterpret_cast<const char *>(p), &cp, &w) == DEC_OK)
        return cp;
      return p[0];
    }

    // A non-empty proper prefix of a well-formed sequence: the bytes
    // [s, end) are the first few bytes of a character that never
    // finishes.  This is the DBCS "invalid MBC = lead byte + NUL" test
    // widened to every UTF-8 width (D9w).  `_s` uses it for EILSEQ,
    // `_mbsrev` and `_mbsnset` for their dud-byte repairs.
    inline bool incomplete_at(const unsigned char *s,
                              const unsigned char *end) noexcept
    {
      if (!s || !end || s >= end)
        return false;
      const unsigned want = seq_width(s[0]);
      if (want <= 1)
        return false; // a one-byte character is never a prefix
      return (size_t)(end - s) < want;
    }

    // The stronger form of `incomplete_at`: every byte after the lead is a
    // trail byte, so [s, end) really is the beginning of one character and
    // not a lead byte followed by something else entirely.  A 0xE3 that is
    // followed by 'a' is a dud either way, but it is not a character the
    // terminator cut short, and `_mbsinc` steps over it one byte at a time
    // rather than abandoning the rest of the string.
    inline bool prefix_of_seq(const unsigned char *s,
                              const unsigned char *end) noexcept
    {
      if (!incomplete_at(s, end))
        return false;
      for (const unsigned char *p = s + 1; p < end; ++p) {
        if (!trail_byte(*p))
          return false;
      }
      return true;
    }

    // ----------------------------------------------------------------
    // code-point folding (plan-3 §5.1: the mb family folds by codepoint,
    // unlike M8's _stricmp byte fold)
    // ----------------------------------------------------------------

    // Lowercase fold of one scalar value.  Only the Basic Multilingual
    // Plane has a wchar_t in this ABI, so above U+FFFF the value is its
    // own fold (documented): the product reaches the whole scalar range
    // through LCMapStringW, our 16-bit towlower does not.
    inline char32_t fold_lower(char32_t cp) noexcept
    {
      if (cp < 0x80)
        return i::u8_byte_lower((unsigned char)cp);
      if (cp > 0xFFFF)
        return cp;
      return (char32_t)i::u8_wfold_lower((wint_t)cp);
    }

    inline char32_t fold_upper(char32_t cp) noexcept
    {
      if (cp < 0x80)
        return i::u8_byte_upper((unsigned char)cp);
      if (cp > 0xFFFF)
        return cp;
      return (char32_t)i::u8_wfold_upper((wint_t)cp);
    }

    // ----------------------------------------------------------------
    // comparison (D9f..D9j)
    // ----------------------------------------------------------------

    enum : int
    {
      CMP_RAW = 0, // code-point order
      CMP_FOLD = 1 // lowercase-folded code-point order
    };

    enum : int
    {
      // Both operands run to their terminator.  A terminator compares
      // less than every other character.
      BND_STR = 0,
      // n counts characters: each side stops after n characters or at its
      // terminator, whichever comes first.
      BND_CHARS = 1,
      // n counts bytes: a character the bound would split reads as the
      // terminator, which is the reference's "naked lead" shape (D9h).
      BND_BYTES = 2
    };

    inline char32_t cmp_key(char32_t cp, int mode) noexcept
    {
      return mode == CMP_FOLD ? fold_lower(cp) : cp;
    }

    // Shared body of the six compare faces.  Validation order follows
    // the reference: a zero count answers 0 before the pointers are
    // looked at, and a null operand is EINVAL + _NLSCMPERROR.
    //
    // The reference spends one shared countdown across both sides, which
    // is exact while the two agree (they must, or the function has
    // already returned) and only observable once a bound lands inside a
    // character.  Each side gets its own byte tally here, so a bound
    // that splits a character on one side and not the other cannot make
    // the two sides run out at different points.
    inline int mbs_compare(const unsigned char *s1,
                           const unsigned char *s2,
                           size_t n,
                           int mode,
                           int bound) noexcept
    {
      if (bound != BND_STR && n == 0)
        return 0;
      if (!s1 || !s2) {
        _set_errno(EINVAL);
        return INT_MIN; // _NLSCMPERROR
      }

      const unsigned char *p1 = s1;
      const unsigned char *p2 = s2;
      size_t used1 = 0; // bytes taken from s1 (byte bound only)
      size_t used2 = 0;
      size_t chars = 0;

      for (;;) {
        if (bound == BND_CHARS && chars == n)
          return 0;

        char32_t c1 = 0;
        char32_t c2 = 0;
        unsigned w1 = 0;
        unsigned w2 = 0;
        if (*p1) {
          w1 = step(p1);
          if (bound != BND_BYTES || used1 + w1 <= n)
            c1 = scalar_at(p1);
          else
            w1 = 0; // the bound would split this character
        }
        if (*p2) {
          w2 = step(p2);
          if (bound != BND_BYTES || used2 + w2 <= n)
            c2 = scalar_at(p2);
          else
            w2 = 0;
        }

        const char32_t k1 = cmp_key(c1, mode);
        const char32_t k2 = cmp_key(c2, mode);
        if (k1 != k2)
          return k1 < k2 ? -1 : 1;
        if (k1 == 0)
          return 0;

        p1 += w1;
        p2 += w2;
        used1 += w1;
        used2 += w2;
        ++chars;
      }
    }

    // ----------------------------------------------------------------
    // locate (D9k..D9n)
    // ----------------------------------------------------------------

    // Charset membership, the one test the four scan faces share.  The
    // set is walked a whole character at a time, so a delimiter is a
    // delimiter in the UTF-8 sense and not a byte that happens to appear
    // inside one.  An incomplete entry at the end of the set is skipped:
    // the reference's "a lead byte followed by NUL matches anything" is
    // an artifact of a dud DBCS pair, not a rule worth carrying over.
    inline bool in_charset(char32_t cp, const unsigned char *set) noexcept
    {
      for (const unsigned char *p = set; *p;) {
        char32_t setcp;
        unsigned setw;
        if (decode(reinterpret_cast<const char *>(p), &setcp, &setw) != DEC_OK)
          break; // malformed or truncated: nothing left to match
        if (setcp == cp)
          return true;
        p += setw;
      }
      return false;
    }

    // _mbschr / _mbsrchr: c is a whole scalar value, the UTF-8 analogue
    // of the DBCS 16-bit code the reference compares against.  A
    // character equal to 0 matches the terminator, which is how the
    // reference handles an embedded-NUL search.
    inline const unsigned char *
    mbs_find(const unsigned char *s, char32_t c, bool last) noexcept
    {
      if (!s) {
        _set_errno(EINVAL);
        return nullptr;
      }

      const unsigned char *hit = nullptr;
      for (const unsigned char *p = s; *p; p += step(p)) {
        if (scalar_at(p) == c) {
          hit = p;
          if (!last)
            return p;
        }
      }
      // The loop never examines the terminator, so a search for 0 can only
      // ever end there.  That is the reference's whole answer to
      // "find the terminator", on both the ordinary path and the
      // `else if (!r) r = str` path it takes when a dud lead byte sits in
      // front of it.
      if (c == 0)
        return s + strlen(reinterpret_cast<const char *>(s));
      return hit;
    }

    // _mbsstr: byte equality over whole characters, tried only at
    // character starts.  The reference compares DBCS units, which is
    // character equality; over UTF-8 the same test is byte equality, so
    // the two agree by construction rather than by luck.
    inline const unsigned char *mbs_strstr(const unsigned char *s,
                                           const unsigned char *sub) noexcept
    {
      // The reference checks the needle first, answers an empty needle
      // with the haystack before it ever looks at the haystack, and only
      // then rejects a null haystack.  So `_mbsstr(nullptr, "")` is
      // nullptr with errno untouched, which is one of the three answers
      // the shape buys and it is not the intuitive one.
      if (!sub) {
        _set_errno(EINVAL);
        return nullptr;
      }
      if (!*sub)
        return s;
      if (!s) {
        _set_errno(EINVAL);
        return nullptr;
      }

      const size_t sublen = strlen(reinterpret_cast<const char *>(sub));
      for (const unsigned char *p = s; *p; p += step(p)) {
        if (strlen(reinterpret_cast<const char *>(p)) < sublen)
          break; // what is left of s is shorter than the needle
        if (memcmp(p, sub, sublen) == 0)
          return p;
      }
      return nullptr;
    }

    // _mbsspn / _mbsspnp: the byte offset of, respectively a pointer to,
    // the first character of s the charset does not contain.
    inline size_t mbs_spn(const unsigned char *s,
                          const unsigned char *set) noexcept
    {
      if (!s || !set) {
        _set_errno(EINVAL);
        return 0;
      }
      const unsigned char *p = s;
      for (; *p; p += step(p))
        if (!in_charset(scalar_at(p), set))
          break;
      return (size_t)(p - s);
    }

    inline const unsigned char *mbs_spnp(const unsigned char *s,
                                         const unsigned char *set) noexcept
    {
      if (!s || !set) {
        _set_errno(EINVAL);
        return nullptr;
      }
      for (const unsigned char *p = s; *p; p += step(p))
        if (!in_charset(scalar_at(p), set))
          return p;
      return nullptr;
    }

    // _mbscspn: the complement of mbs_spn, as _mbspbrk is the complement
    // of _mbsspnp.
    inline size_t mbs_cspn(const unsigned char *s,
                           const unsigned char *set) noexcept
    {
      if (!s || !set) {
        _set_errno(EINVAL);
        return 0;
      }
      const unsigned char *p = s;
      for (; *p; p += step(p))
        if (in_charset(scalar_at(p), set))
          break;
      return (size_t)(p - s);
    }

    // _mbspbrk: the first character the charset *does* contain.
    inline const unsigned char *mbs_pbrk(const unsigned char *s,
                                         const unsigned char *set) noexcept
    {
      if (!s || !set) {
        _set_errno(EINVAL);
        return nullptr;
      }
      for (const unsigned char *p = s; *p; p += step(p))
        if (in_charset(scalar_at(p), set))
          return p;
      return nullptr;
    }

    // ----------------------------------------------------------------
    // the _s protocol (D9v)
    // ----------------------------------------------------------------
    // _RESET_STRING: terminate and zero the whole buffer.  Every failure
    // path the reference shares does this before reporting, so a caller
    // that ignores the return value still cannot read a stale string.
    //
    // _CRT_UNBOUNDED_BUFFER_SIZE is (size_t)-1 and the reference's own
    // non-secure `_mbslwr` / `_mbsupr` pass it, so "zero the caller's
    // buffer" has to have a defined answer when there is no caller buffer
    // to name.  There is none: the answer is nothing to do.  Without this
    // guard an unbounded size would memset four billion bytes, which is
    // the difference between a documented no-op and a very large mistake.
    inline void reset_string(unsigned char *dst, size_t size) noexcept
    {
      if (dst && size > 0 && size != (size_t)-1)
        memset(dst, 0, size);
    }

    // _FILL_STRING: zero everything past what was written.  Same
    // unbounded case as reset_string, for the same reason.
    inline void
    fill_string(unsigned char *dst, size_t size, size_t used) noexcept
    {
      if (!dst || used >= size || size == (size_t)-1)
        return;
      memset(dst + used, 0, size - used);
    }

    // _VALIDATE_STRING
    inline bool valid_string(const unsigned char *dst, size_t size) noexcept
    {
      if (dst && size > 0)
        return true;
      _set_errno(EINVAL);
      return false;
    }

    // The whole-string form the ncat/ncpy secure wrappers share: a zero
    // count with a null destination and a zero size is allowed and does
    // nothing.
    inline bool count_zero_noop(const unsigned char *dst,
                                size_t size,
                                size_t count) noexcept
    {
      return count == 0 && !dst && size == 0;
    }

    // ----------------------------------------------------------------
    // copy / concatenate (D9q, D9r)
    // ----------------------------------------------------------------

    // Copies whole characters from src to dst, stopping at dst's byte
    // budget or src's terminator, whichever comes first.  *out_used gets
    // the bytes written and the return value is the address just past
    // them.
    //
    // char_budget == (size_t)-1 means unbounded, which is how the
    // `_mbsn*` / `_mbsnb*` split is expressed: `_mbsncpy` bounds by
    // characters and `_mbsnbcpy` by bytes, and both go through here so
    // the two cannot drift into the same function.
    // Bytes of src the two budgets select, whole characters only.  The
    // char_budget form is `_mbsn*` and the byte_budget form is `_mbsnb*`;
    // (size_t)-1 means unbounded.  Measuring the span before anything is
    // written is what lets the secure forms answer ERANGE without having
    // overrun the caller's buffer to find out.
    inline size_t span_bytes(const unsigned char *src,
                             size_t char_budget,
                             size_t byte_budget) noexcept
    {
      size_t used = 0;
      for (size_t n = 0;; ++n) {
        if (char_budget != (size_t)-1 && n >= char_budget)
          break;
        if (!src[used])
          break;
        const unsigned w = step(src + used);
        if (byte_budget != (size_t)-1 && used + w > byte_budget)
          break; // never split a character at the boundary
        used += w;
      }
      return used;
    }

    inline unsigned char *copy_chars(unsigned char *dst,
                                     const unsigned char *src,
                                     size_t char_budget,
                                     size_t byte_budget,
                                     size_t *out_used) noexcept
    {
      const size_t used = span_bytes(src, char_budget, byte_budget);
      if (used)
        memcpy(dst, src, used);
      *out_used = used;
      return dst + used;
    }

    // Copies at most count whole characters and reports both tallies.
    // The non-secure `_mbsn*` forms need the character count and not just
    // the byte count, because their padding rule is "fill the destination
    // out to count characters" -- the reference's `while (cnt--)
    // *dst++ = '\0'` over the leftover count.
    inline void fill_n(unsigned char *dst,
                       const unsigned char *src,
                       size_t count,
                       size_t *used,
                       size_t *chars) noexcept
    {
      size_t u = 0;
      size_t n = 0;
      while (n < count && src[u]) {
        const unsigned w = step(src + u);
        memcpy(dst + u, src + u, w);
        u += w;
        ++n;
      }
      *used = u;
      *chars = n;
    }

    // ----------------------------------------------------------------
    // case conversion (D9x)
    // ----------------------------------------------------------------

    // Folds a whole string in place and returns the number of bytes the
    // result needs.  A fold that would change a character's encoded
    // width (U+212A KELVIN SIGN -> U+006B is the classic case) cannot be
    // done in place, so that character is left alone and *blocked is
    // raised: the reference reports the same situation as its
    // LCMapStringA failure, errno = EILSEQ.  Narrowing is always safe, so
    // a shorter result needs no complaint.
    inline size_t fold_in_place(unsigned char *s,
                                size_t limit,
                                int up,
                                bool *blocked) noexcept
    {
      unsigned char tmp[4];
      size_t read = 0;
      size_t write = 0;
      *blocked = false;

      while (read < limit && s[read]) {
        const unsigned char *p = s + read;
        const char32_t cp = scalar_at(p);
        const unsigned w = step(p);

        // A byte that does not start a character is the reference's
        // "naked lead byte", i.e. a malformed MBC, which its fold call
        // rejected with EILSEQ.
        if (!seq_ok(p)) {
          *blocked = true;
          return read;
        }

        mbstate_t st;
        memset(&st, 0, sizeof(st));
        const size_t enc = __c32rtomb_utf8(reinterpret_cast<char *>(tmp),
                                           up ? fold_upper(cp) : fold_lower(cp),
                                           &st);
        if (enc == (size_t)-1 || enc == 0 || enc > w) {
          *blocked = true;
          return read;
        }

        // Always write.  The naive guard is "skip when the write position
        // has caught up with the read position", which is right only if
        // the fold is a no-op -- and the commonest fold of all, a letter
        // changing case, keeps the character exactly as wide as it was.  So
        // the guard made _mbslwr("AbC") return 0 having folded nothing.
        // Four bytes of scratch and a memmove that is right by
        // construction beat a comparison that has to be kept in step with
        // what the fold tables contain.
        memmove(s + write, tmp, enc);
        write += enc;
        read += w;
      }
      return write;
    }

    // ----------------------------------------------------------------
    // tokenize (D9z)
    // ----------------------------------------------------------------

    // The body shared by `_mbstok_s` and the static-context `_mbstok`.
    // Delimiters are whole characters and a token always begins and ends
    // on a character boundary.  *ctx moves to just past the delimiter
    // that ended the token, or to the terminator when the token ran to
    // the end of the string.
    inline unsigned char *tok_split(unsigned char *s,
                                    const unsigned char *set,
                                    unsigned char **ctx) noexcept
    {
      if (!ctx || !set || (!s && !*ctx)) {
        _set_errno(EINVAL);
        return nullptr;
      }
      if (!s)
        s = *ctx;

      unsigned char *p = s;
      for (; *p; p += step(p)) {
        if (!in_charset(scalar_at(p), set))
          break;
      }
      if (!*p) {
        *ctx = p;
        return nullptr; // no token at all
      }

      unsigned char *token = p;
      for (; *p; p += step(p)) {
        if (in_charset(scalar_at(p), set)) {
          // Blank the delimiter's first byte and resume after the whole
          // character, so the context stays aligned.
          const unsigned w = step(p);
          *p = '\0';
          *ctx = p + w;
          return token;
        }
      }
      *ctx = p; // the terminator: the enumeration is finished
      return token;
    }

    // ----------------------------------------------------------------
    // counting (D9d, D9e, D9o, D9p)
    // ----------------------------------------------------------------

    // A lead byte whose character the terminator (or a bound) cuts short.
    // This is the one test the reference's counting loops all share: a
    // *lead* means a character was started and never finished, so the walk
    // stops there and the dud byte is not counted.  It is stricter than
    // step()'s "malformed advances one byte" recovery, and it is the only
    // reading that reproduces the shells on both cases at once --
    // `"a\xE3"` is 1 character, not 2, and `"a\x81z"` is 3, not 1.
    inline bool tail_cut(const unsigned char *p) noexcept
    {
      return seq_width(p[0]) > 1 && !seq_ok(p);
    }

    // _mbsnccnt: whole characters inside the first bcnt bytes.  A
    // character the bound would split is not counted and ends the walk
    // (the reference's `!bcnt--` break); a cut tail ends it the same way.
    inline size_t mbs_nccnt(const unsigned char *s, size_t bcnt) noexcept
    {
      if (!s && bcnt != 0) {
        _set_errno(EINVAL);
        return 0;
      }
      size_t n = 0;
      for (size_t used = 0; used < bcnt && s[used];) {
        const unsigned w = step(s + used);
        if (used + w > bcnt)
          break; // the bound would split a character
        if (tail_cut(s + used))
          break; // a character the terminator cuts short
        used += w;
        ++n;
      }
      return n;
    }

    // _mbsnbcnt: bytes occupied by the first ccnt characters.  Never
    // splits a character and never counts a cut tail — the reference's
    // `--p; break` repair, which leaves the dud lead byte out of the
    // total.
    inline size_t mbs_nbcnt(const unsigned char *s, size_t ccnt) noexcept
    {
      if (!s && ccnt != 0) {
        _set_errno(EINVAL);
        return 0;
      }
      size_t n = 0;
      size_t used = 0;
      while (n < ccnt && s[used]) {
        if (tail_cut(s + used))
          break;
        used += step(s + used);
        ++n;
      }
      return used;
    }

    // _mbslen: characters in the whole string.  The reference shell's
    // own comment settles the unit — "Find the length of the MBCS string
    // (in characters)" — and its loop is the tail_cut rule above, so a
    // truncated tail is not counted.  This is *not* the same function as
    // _mbstrlen, which validates the whole window and reports EILSEQ.
    inline size_t mbs_len(const unsigned char *s) noexcept
    {
      if (!s) {
        _set_errno(EINVAL);
        return 0;
      }
      size_t n = 0;
      for (const unsigned char *p = s; *p;) {
        if (tail_cut(p))
          break;
        p += step(p);
        ++n;
      }
      return n;
    }

    // _mbsnlen: the first min(count, length) characters.  The reference
    // has no validation section at all, so the null guard here is our own
    // hardening -- a native null deref is not a behaviour worth copying.
    inline size_t mbs_nlen(const unsigned char *s, size_t count) noexcept
    {
      if (!s) {
        _set_errno(EINVAL);
        return 0;
      }
      size_t n = 0;
      for (const unsigned char *p = s; n < count && *p;) {
        if (tail_cut(p))
          break;
        p += step(p);
        ++n;
      }
      return n;
    }

    // _mbstrlen / _mbstrnlen, from reference/ucrt/convert/_mbslen.cpp's
    // common_mbstrlen_l (D9p).  Two things distinguish this pair from
    // _mbslen / _mbsnlen above: the max_size is a *byte* bound that the
    // answer is clipped to, and the whole inspected window is validated
    // before it is counted.  The return is a character count when the
    // string ends inside the window, max_size itself when the bound cuts
    // it, and (size_t)-1 with EILSEQ for anything malformed.
    //
    // The validation is the reference's
    // MultiByteToWideChar(MB_ERR_INVALID_CHARS, string, (int)max_size, ...)
    // reimplemented over bytes: a sequence the bound or the terminator
    // leaves incomplete is exactly what that call rejects, which is why
    // `_mbstrnlen("a\xE3\x81\x82", 2)` is EILSEQ and not 1.
    inline size_t mbs_strlen(const char *s, size_t max_size) noexcept
    {
      if (!s) {
        _set_errno(EINVAL);
        return (size_t)-1;
      }

      // The window: the first min(length, max_size) bytes.
      size_t window = strlen(s);
      if (max_size < window)
        window = max_size;

      // Pass one, validate.  Nothing is counted yet, so a malformed
      // sequence anywhere in the window answers before any partial answer
      // can be observed.
      for (size_t used = 0; used < window;) {
        const unsigned w =
            seq_ok(reinterpret_cast<const unsigned char *>(s) + used);
        if (!w || used + w > window) {
          _set_errno(EILSEQ);
          return (size_t)-1;
        }
        used += w;
      }

      // Pass two, count.  The reference's `++size; ++it` trail step is
      // generalised from one trail byte to width-1 of them, so the byte
      // tally and the character tally stay the reference's own arithmetic
      // with the DBCS pair replaced by a UTF-8 sequence.
      size_t n = 0;
      size_t size = 0;
      while (size < max_size) {
        if (!s[size])
          break;
        size += step(reinterpret_cast<const unsigned char *>(s) + size);
        ++n;
      }
      return size >= max_size ? max_size : n;
    }

    // _mbstrnlen adds the two argument checks the reference puts in
    // _mbstrnlen_l rather than in the shared body: a null string and a
    // max_size above INT_MAX.  (The cast to int inside the shared body is
    // what makes the ceiling a real constraint and not a formality.)
    // _mbstrlen bypasses both, exactly as the reference does -- it calls
    // the body with _CRT_UNBOUNDED_BUFFER_SIZE without validating.
    inline size_t mbs_strnlen(const char *s, size_t max_size) noexcept
    {
      if (!s) {
        _set_errno(EINVAL);
        return (size_t)-1;
      }
      if (max_size > (size_t)INT_MAX) {
        _set_errno(EINVAL);
        return (size_t)-1;
      }
      return mbs_strlen(s, max_size);
    }

    // ----------------------------------------------------------------
    // pointer walks (D9a..D9c)
    // ----------------------------------------------------------------

    // _mbsnextc: the scalar at p, 0 for a malformed or truncated one.
    inline unsigned mbs_nextc(const unsigned char *p) noexcept
    {
      if (!p) {
        _set_errno(EINVAL);
        return 0;
      }
      if (!seq_ok(p))
        return 0;
      return (unsigned)scalar_at(p);
    }

    // _mbsinc: one character forward.  The reference's flat "+2 for a
    // lead" is a DBCS-pair assumption; under UTF-8 the lead byte's
    // declared width is the answer, so a three-byte character advances
    // three (plan-3 §5.1).  A truncated tail advances to the terminator,
    // which is what the reference's "no second step at EOS" does too.
    inline const unsigned char *mbs_inc(const unsigned char *p) noexcept
    {
      if (!p) {
        _set_errno(EINVAL);
        return nullptr;
      }
      const unsigned w = seq_ok(p);
      if (w)
        return p + w;

      // A character the terminator cuts short steps to the terminator, which
      // is the reference's "the second byte was the end of the string, so
      // do not take the second step" -- a lead byte plus a terminator is
      // exactly the invalid MBC the reference is guarding against, and the
      // pointer it hands back is the end of the string.  Advancing the
      // declared width would land past the terminator; advancing one byte
      // would land *inside* the dud, which is a place a caller cannot do
      // anything with.
      if (*p) {
        const unsigned char *end = p;
        while (*end)
          ++end;
        if (prefix_of_seq(p, end))
          return end;
      }

      // Anything else that is not a character start -- a stray continuation
      // byte, a lead byte whose next byte is not a trail byte -- advances a
      // single byte, which keeps the function total (D9a).
      return p + 1;
    }

    // _mbsdec: the start of the character before the one that ends just
    // below current.  Walked forward rather than guessed backwards: a
    // backwards scan has to re-derive the alignment, and the two answers
    // can differ on a malformed string, where a guess is exactly what a
    // caller cannot check.
    inline const unsigned char *mbs_dec(const unsigned char *s,
                                        const unsigned char *current) noexcept
    {
      if (!s || !current) {
        _set_errno(EINVAL);
        return nullptr;
      }
      if (s >= current)
        return nullptr; // nothing precedes the start; errno untouched

      // The character that contains current - 1 *is* the answer, which is
      // the reference's own arithmetic read carefully: with `current` on a
      // character's first byte, `current - 1` is the byte before it, so the
      // character containing it is the one that ends just before current.
      // With `current` inside a character, `current - 1` is that same
      // character, so the answer is the start of the character the caller
      // pointed into -- which is what stops a half-character being handed
      // back.  There is no "previous" walk to do on top of it.
      const unsigned char *here = walk_to(s, current - 1);
      return here ? const_cast<unsigned char *>(here) : nullptr;
    }

    // ----------------------------------------------------------------
    // set / fill (D9s..D9u)
    // ----------------------------------------------------------------

    // Writes val at s for the region the two budgets select, in place,
    // replacing a whole character at a time.  *dud is raised when val
    // cannot be encoded or would leave a half sequence behind; the
    // reference repairs the same situation by writing a space and
    // setting errno EINVAL, which is what the callers turn into the
    // return value.
    inline unsigned char *set_chars(unsigned char *s,
                                    char32_t val,
                                    size_t char_budget,
                                    size_t byte_budget,
                                    bool *dud) noexcept
    {
      unsigned char rep[4];
      mbstate_t st;
      memset(&st, 0, sizeof(st));
      const size_t repw =
          __c32rtomb_utf8(reinterpret_cast<char *>(rep), val, &st);
      if (repw == (size_t)-1 || repw == 0) {
        // an unencodable val is the reference's dud MBC pair: it fills
        // with spaces rather than refusing, and reports EINVAL
        rep[0] = ' ';
        rep[1] = 0;
      }

      *dud = false;
      size_t used = 0;
      for (size_t n = 0; char_budget == (size_t)-1 || n < char_budget;) {
        // The terminator is read at the *cursor*, not at s: s never moves
        // (the writes go to s + used), so `!*s` is true only when the
        // string is already empty, and every unbounded fill -- _mbsset,
        // which has no count to stop it -- would run off the end of the
        // caller's buffer writing the fill value over whatever followed.
        if (!s[used])
          break;
        // A byte budget that is used up exactly ends the walk cleanly.
        // Only a character that *straddles* the budget is blanked, which
        // is the reference's `!count--` sitting inside its loop: the
        // decrement that reads zero stops the loop before the body, and
        // the decrement that reads zero in the body is the one byte of
        // room it was short.  Without the split, an all-ASCII string
        // would have its first byte past the count blanked.
        if (byte_budget != (size_t)-1 && used >= byte_budget)
          break;
        const unsigned w = step(s + used);
        if (byte_budget != (size_t)-1 && used + w > byte_budget) {
          // The reference writes a single space here and stops, "pad with
          // ' ' if no room for both bytes".  Over DBCS that leaves at most
          // one orphaned trail byte, which the codepage treats as noise;
          // over UTF-8 it would leave two or three bytes no lead byte
          // claims, and every other face in this layer treats that as a
          // malformed string.  So the whole character is blanked instead
          // of its first byte: spaces are still what the reference writes,
          // the string is still terminated where it was, and it still
          // decodes.  `w` bytes are always inside the string, because
          // step() returned w only after reading all w of them.
          memset(s + used, ' ', w);
          *dud = true;
          break;
        }
        if (w != repw) {
          // the replacement is a different width: pad the tail of the
          // character being replaced so no half sequence survives
          memset(s + used, ' ', w);
          *dud = true;
        } else {
          memcpy(s + used, rep, repw);
        }
        used += w;
        if (char_budget != (size_t)-1)
          ++n;
      }
      return s;
    }

    // ----------------------------------------------------------------
    // reverse (D9y)
    // ----------------------------------------------------------------

    // Reverses by whole characters, in the reference's own two phases, and
    // the order is not interchangeable: reversing the whole byte string
    // first would leave characters that no longer decode, and the second
    // phase would have no way to find their boundaries.
    //
    // Phase one reverses the bytes *inside* each character, which leaves
    // every boundary where it was, so the walk reaches the next character.
    // Phase two is a plain byte sweep of the whole string, and by then the
    // characters stand in mirror order with their own bytes back in order.
    // That is also the only reason a mixed-width string reverses at all:
    // exchanging the two ends directly cannot work when the ends are
    // different widths, because the middle would have to move with them.
    //
    // A lead byte the terminator cuts short is the reference's unsolvable
    // case -- reversing it would attach the lead byte to the character
    // before it -- so the string is truncated there and EINVAL reported,
    // exactly as the reference repairs itself.
    inline unsigned char *mbs_rev(unsigned char *s) noexcept
    {
      if (!s) {
        _set_errno(EINVAL);
        return nullptr;
      }

      unsigned char *p = s;
      while (*p) {
        const unsigned w = seq_ok(p);
        if (!w) {
          *p = '\0'; // drop the incomplete character
          _set_errno(EINVAL);
          break;
        }
        for (unsigned i = 0; i < w / 2; ++i) {
          const unsigned char t = p[i];
          p[i] = p[w - 1 - i];
          p[w - 1 - i] = t;
        }
        p += w;
      }
      unsigned char *end = p; // one past the last character, either way

      for (unsigned char *lo = s, *hi = end - 1; lo < hi; ++lo, --hi) {
        const unsigned char t = *lo;
        *lo = *hi;
        *hi = t;
      }
      return s;
    }

    // ----------------------------------------------------------------
    // the secure copy / concatenate family (D9v)
    // ----------------------------------------------------------------

    // First byte of the last character of a NUL-terminated string.  Used
    // by the EILSEQ tests, which are about the final character and not
    // about the string as a whole.
    inline const unsigned char *last_char_start(const unsigned char *s,
                                                size_t n) noexcept
    {
      if (n == 0)
        return s;
      const size_t back = back_off(s, s + n - 1);
      return s + n - 1 - back;
    }

    // Offset of a trailing character that never finishes, or (size_t)-1
    // when the copied region ends on a character boundary.  When one is
    // there, the reference clears the copied lead byte and calls it a
    // malformed MBC, so dst ends up holding the string minus its last
    // character.
    //
    // The offset is measured from the character start rather than
    // computed by subtracting the lead byte's declared width, because a
    // cut sequence is by definition *shorter* than the width its lead
    // byte announces: `_mbscpy_s(dst, n, "a\xE3")` copies two bytes, and
    // 2 - 3 is not an offset.  Reading the start back off the string is
    // the only side of that subtraction that can be trusted.
    inline size_t copy_tail_dud(const unsigned char *src, size_t used) noexcept
    {
      const unsigned char *start = last_char_start(src, used);
      if (!incomplete_at(start, src + used))
        return (size_t)-1;
      return (size_t)(start - src);
    }

    // The secure copy body.  `count` is the caller's own count and
    // count_in_bytes picks the family's n / nb convention: the `_mbsn*`
    // forms count characters, the `_mbsnb*` forms count bytes, and
    // _mbscpy_s passes (size_t)-1 for "no count at all".
    inline errno_t ncopy_s(unsigned char *dst,
                           size_t size,
                           const unsigned char *src,
                           size_t count,
                           bool count_in_bytes) noexcept
    {
      if (!valid_string(dst, size))
        return EINVAL;
      if (count == 0) {
        // the reference resets the destination and reports success; the
        // source pointer is not even looked at
        reset_string(dst, size);
        return 0;
      }
      if (!src) {
        reset_string(dst, size);
        _set_errno(EINVAL);
        return EINVAL;
      }

      const size_t take = span_bytes(src,
                                     count_in_bytes ? (size_t)-1 : count,
                                     count_in_bytes ? count : (size_t)-1);
      if (take + 1 > size) {
        reset_string(dst, size);
        _set_errno(ERANGE);
        return ERANGE;
      }
      if (take)
        memcpy(dst, src, take);
      const size_t dud = copy_tail_dud(src, take);
      if (dud != (size_t)-1) {
        dst[dud] = '\0';
        _set_errno(EILSEQ);
        return EILSEQ;
      }

      dst[take] = '\0';
      fill_string(dst, size, take + 1);
      return 0;
    }

    inline errno_t
    cpy_s(unsigned char *dst, size_t size, const unsigned char *src) noexcept
    {
      return ncopy_s(dst, size, src, (size_t)-1, false);
    }

    // The secure concatenate body, same count convention as ncopy_s.
    inline errno_t ncat_s(unsigned char *dst,
                          size_t size,
                          const unsigned char *src,
                          size_t count,
                          bool count_in_bytes) noexcept
    {
      if (count == 0 && count_zero_noop(dst, size, count))
        return 0;
      if (!valid_string(dst, size))
        return EINVAL;

      size_t used = 0;
      while (used < size && dst[used])
        ++used;
      if (used == size) {
        // the reference cannot look past the buffer to tell whether dst
        // ended with a dud character, so it reports and resets
        reset_string(dst, size);
        _set_errno(EINVAL);
        return EINVAL;
      }
      if (used && incomplete_at(last_char_start(dst, used), dst + used)) {
        // dst ended in an incomplete character: drop it rather than glue
        // the source's first byte onto it.  The start is read back off the
        // string for the same reason copy_tail_dud does it -- the cut
        // sequence is shorter than the width its lead byte announces.
        const unsigned char *tail = last_char_start(dst, used);
        used = (size_t)(tail - dst);
        dst[used] = '\0';
      }

      if (count != 0 && !src) {
        reset_string(dst, size);
        _set_errno(EINVAL);
        return EINVAL;
      }

      size_t add = 0;
      if (count != 0)
        add = span_bytes(src,
                         count_in_bytes ? (size_t)-1 : count,
                         count_in_bytes ? count : (size_t)-1);

      if (used + add + 1 > size) {
        reset_string(dst, size);
        _set_errno(ERANGE);
        return ERANGE;
      }
      if (add)
        memcpy(dst + used, src, add);
      const size_t dud = copy_tail_dud(src, add);
      if (dud != (size_t)-1) {
        dst[used + dud] = '\0';
        _set_errno(EILSEQ);
        return EILSEQ;
      }

      dst[used + add] = '\0';
      fill_string(dst, size, used + add + 1);
      return 0;
    }

    inline errno_t
    cat_s(unsigned char *dst, size_t size, const unsigned char *src) noexcept
    {
      return ncat_s(dst, size, src, (size_t)-1, false);
    }

    // ----------------------------------------------------------------
    // the secure case / set family (D9v)
    // ----------------------------------------------------------------

    // _mbslwr_s / _mbsupr_s.  The validation shape is wine-anchored
    // (lwr_s("AbC",4)==0, lwr_s("AbC",3)==EINVAL with the buffer reset,
    // lwr_s("AbC",0)==EINVAL with the buffer untouched, lwr_s(NULL,0)==0,
    // lwr_s(NULL,4)==EINVAL) and a fold can only narrow a character, so
    // the capacity the reference checks up front is all we can need.
    inline errno_t case_s(unsigned char *s, size_t size, int up) noexcept
    {
      if (!((s && size > 0) || (!s && size == 0))) {
        _set_errno(EINVAL);
        return EINVAL;
      }
      if (!s)
        return 0; // nothing to do

      size_t len = 0;
      while (len < size && s[len])
        ++len;
      if (len >= size) {
        reset_string(s, size);
        _set_errno(EINVAL);
        return EINVAL;
      }
      fill_string(s, size, len + 1);

      bool blocked = false;
      const size_t out = fold_in_place(s, len, up, &blocked);
      s[out] = '\0';
      fill_string(s, size, out + 1);
      if (blocked) {
        reset_string(s, size);
        _set_errno(EILSEQ);
        return EILSEQ;
      }
      return 0;
    }

    // _mbsset_s / _mbsnset_s / _mbsnbset_s, same count convention as
    // ncopy_s (count == (size_t)-1 for "the whole string").  The
    // reference's "value is not a valid mbchar" precheck becomes the
    // unencodable-value arm of set_chars, and its odd-count repair for a
    // two-byte value becomes the byte-bound form's space pad.
    inline errno_t set_s(unsigned char *dst,
                         size_t size,
                         char32_t val,
                         size_t count,
                         bool count_in_bytes) noexcept
    {
      if (count == 0 && count_zero_noop(dst, size, count))
        return 0;
      if (!valid_string(dst, size))
        return EINVAL;

      size_t len = 0;
      while (len < size && dst[len])
        ++len;
      if (len == size) {
        reset_string(dst, size);
        _set_errno(EINVAL);
        return EINVAL;
      }

      bool dud = false;
      if (count != 0) {
        set_chars(dst,
                  val,
                  count_in_bytes ? (size_t)-1 : count,
                  count_in_bytes ? count : (size_t)-1,
                  &dud);
        if (dud) {
          // the reference terminates the string where it could not finish
          // the value, so what is left is still a well-formed string
          while (len < size && dst[len])
            ++len;
          dst[len] = '\0';
          fill_string(dst, size, len + 1);
          _set_errno(EILSEQ);
          return EILSEQ;
        }
      }

      fill_string(dst, size, len + 1);
      return 0;
    }

    // ----------------------------------------------------------------
    // duplicate (D9z)
    // ----------------------------------------------------------------

    // _mbsdup: malloc + copy.  A null string answers null and leaves
    // errno alone (wine anchor); an empty string still allocates, so the
    // caller can free unconditionally.
    inline unsigned char *mbs_dup(const unsigned char *s) noexcept
    {
      if (!s)
        return nullptr;
      const size_t n = strlen(reinterpret_cast<const char *>(s)) + 1;
      unsigned char *copy = static_cast<unsigned char *>(malloc(n));
      if (copy)
        memcpy(copy, s, n);
      return copy;
    }
  } // namespace mbstring
} // namespace mingw_thunk
