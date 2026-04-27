#include <iconv.h>

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "descriptor.h"
#include "encoding.h"

namespace libiconv
{
  static bool is_utf16_family(unsigned int cp)
  {
    return cp == 1200 || cp == 1201;
  }

  static bool is_utf32_family(unsigned int cp)
  {
    return cp == 12000 || cp == 12001;
  }

  // Determine the byte length of the next source character for MB encodings.
  // Returns 1-4. Does not validate the sequence — invalid sequences are caught
  // by MultiByteToWideChar with MB_ERR_INVALID_CHARS.
  static int source_char_len(unsigned int cp, const uint8_t *src, size_t remaining)
  {
    if (remaining == 0)
      return 1;

    uint8_t b = src[0];

    // UTF-8 (CP 65001)
    if (cp == 65001) {
      if (b < 0x80) return 1;
      if (b < 0xC0) return 1;  // continuation byte — invalid as lead
      if (b < 0xC2) return 1;  // 0xC0-0xC1 overlong — invalid
      if (b < 0xE0) return 2;  // 0xC2-0xDF: valid 2-byte lead
      if (b < 0xF0) return 3;  // 0xE0-0xEF: valid 3-byte lead
      if (b < 0xF5) return 4;  // 0xF0-0xF4: valid 4-byte lead
      return 1;                // 0xF5-0xFF: invalid
    }

    // GB18030 (CP 54936)
    if (cp == 54936) {
      if (b < 0x80) return 1;
      if (remaining >= 2 && src[1] >= 0x30 && src[1] <= 0x39) return 4;
      return 2;
    }

    // DBCS codepages (CP 936, 950, 932, 949, etc.)
    if (IsDBCSLeadByteEx(cp, (char)b))
      return 2;

    // SBCS and everything else
    return 1;
  }

  // Advance input pointers past `char_len` consumed bytes, accounting for
  // bytes that came from pending vs. current input.
  static void advance_input(iconv_descriptor *d,
                            const char **inbuf,
                            size_t *inbytesleft,
                            bool had_pending,
                            int char_len)
  {
    if (had_pending) {
      if ((size_t)char_len >= d->from_pending_len) {
        size_t from_src = char_len - d->from_pending_len;
        if (from_src > *inbytesleft)
          from_src = *inbytesleft;
        *inbuf += from_src;
        *inbytesleft -= from_src;
        d->from_pending_len = 0;
      } else {
        size_t rem = d->from_pending_len - char_len;
        memmove(d->from_pending, d->from_pending + char_len, rem);
        d->from_pending_len = rem;
      }
    } else {
      *inbuf += char_len;
      *inbytesleft -= char_len;
    }
  }

  // Save incomplete character bytes to pending buffer.
  static void save_to_pending(iconv_descriptor *d,
                              const char **inbuf,
                              size_t *inbytesleft,
                              bool had_pending,
                              const uint8_t *cs,
                              size_t cs_len,
                              int char_len)
  {
    if (!had_pending) {
      size_t to_save = cs_len;
      if (to_save > 4) to_save = 4;
      memcpy(d->from_pending, cs, to_save);
      d->from_pending_len = to_save;
      *inbuf += to_save;
      *inbytesleft -= to_save;
    } else {
      // Bytes partly in pending, partly from current input.
      // Add more bytes from current input to reach char_len.
      size_t pending_have = d->from_pending_len;
      size_t src_available = *inbytesleft;
      size_t need = (size_t)char_len - pending_have;
      size_t to_add = need;
      if (to_add > src_available) to_add = src_available;
      if (to_add > 4 - pending_have) to_add = 4 - pending_have;
      if (to_add > 0) {
        memcpy(d->from_pending + pending_have, *inbuf, to_add);
        d->from_pending_len = pending_have + to_add;
        *inbuf += to_add;
        *inbytesleft -= to_add;
      }
    }
  }

  // Encode one logical character (1-2 wchar_t) to the target encoding.
  // Returns 0 on success, -1 on error (sets errno).
  // On IGNORE mode with used_default, output is suppressed but returns 0.
  static int encode_one(iconv_descriptor *d,
                        const wchar_t *wc,
                        int wide_count,
                        char **outbuf,
                        size_t *outbytesleft,
                        size_t *non_reversible)
  {
    unsigned int cp = d->to_cp;

    // --- UTF-16 target ---
    if (is_utf16_family(cp)) {
      size_t needed = (size_t)wide_count * 2;
      if (needed > *outbytesleft) {
        errno = E2BIG;
        return -1;
      }
      if (cp == 1200) {
        memcpy(*outbuf, wc, needed);
      } else {
        uint16_t *dst = (uint16_t *)*outbuf;
        for (int i = 0; i < wide_count; i++)
          dst[i] = __builtin_bswap16((uint16_t)wc[i]);
      }
      *outbuf += needed;
      *outbytesleft -= needed;
      return 0;
    }

    // --- UTF-32 target ---
    if (is_utf32_family(cp)) {
      if (*outbytesleft < 4) {
        errno = E2BIG;
        return -1;
      }

      uint32_t codepoint;
      if (wide_count == 2 && wc[0] >= 0xD800 && wc[0] <= 0xDBFF &&
          wc[1] >= 0xDC00 && wc[1] <= 0xDFFF) {
        codepoint = 0x10000 + (((uint32_t)wc[0] - 0xD800) << 10) +
                    ((uint32_t)wc[1] - 0xDC00);
      } else if (wide_count == 1 && (wc[0] < 0xD800 || wc[0] > 0xDFFF)) {
        codepoint = (uint32_t)wc[0];
      } else {
        // Invalid surrogate sequence
        if (d->discard_ilseq) {
          (*non_reversible)++;
          return 0;
        }
        errno = EILSEQ;
        return -1;
      }

      uint8_t *dst = (uint8_t *)*outbuf;
      if (cp == 12000) {
        dst[0] = (uint8_t)(codepoint);
        dst[1] = (uint8_t)(codepoint >> 8);
        dst[2] = (uint8_t)(codepoint >> 16);
        dst[3] = (uint8_t)(codepoint >> 24);
      } else {
        dst[0] = (uint8_t)(codepoint >> 24);
        dst[1] = (uint8_t)(codepoint >> 16);
        dst[2] = (uint8_t)(codepoint >> 8);
        dst[3] = (uint8_t)(codepoint);
      }
      *outbuf += 4;
      *outbytesleft -= 4;
      return 0;
    }

    // --- MB target encoding ---
    int span = wide_count;

    // Convert directly — same pattern as the old batch code.
    // WideCharToMultiByte writes to the output buffer and signals
    // substitution via lpUsedDefaultChar.  We must NOT use a two-pass
    // (query then convert) approach because Wine may set lpUsedDefaultChar
    // differently depending on whether the output buffer is provided.
    BOOL used_default = FALSE;
    int n = WideCharToMultiByte(cp, 0, wc, span,
                                *outbuf, (int)*outbytesleft,
                                NULL, &used_default);

    if (n > 0) {
      if (used_default) {
        if (d->discard_ilseq) {
          // IGNORE: discard output, count as non-reversible
          (*non_reversible)++;
          return 0;
        }
        if (!d->translit) {
          // Strict: EILSEQ
          errno = EILSEQ;
          return -1;
        }
        // TRANSLIT: keep the '?' replacement, count as non-reversible
        (*non_reversible)++;
      }
      *outbuf += n;
      *outbytesleft -= n;
      return 0;
    }

    // n == 0: conversion failed
    DWORD err = GetLastError();
    if (err == ERROR_INSUFFICIENT_BUFFER) {
      errno = E2BIG;
      return -1;
    }

    // Other errors (invalid character, etc.)
    if (d->discard_ilseq) {
      (*non_reversible)++;
      return 0;
    }
    errno = EILSEQ;
    return -1;
  }

  extern "C" size_t iconv(iconv_t cd,
                          const char **inbuf,
                          size_t *inbytesleft,
                          char **outbuf,
                          size_t *outbytesleft)
  {
    if (cd == (iconv_t)-1) {
      errno = EBADF;
      return (size_t)-1;
    }

    iconv_descriptor *d = (iconv_descriptor *)cd;

    if (!inbuf || !*inbuf) {
      d->from_pending_len = 0;
      d->pending_high_surrogate = 0;
      return 0;
    }

    // Same codepage: direct copy
    if (d->from_cp == d->to_cp) {
      if (!outbuf || !*outbuf || !outbytesleft)
        return 0;
      size_t to_copy = *inbytesleft;
      if (to_copy > *outbytesleft)
        to_copy = *outbytesleft;
      if (to_copy > 0) {
        memcpy(*outbuf, *inbuf, to_copy);
        *inbuf += to_copy;
        *inbytesleft -= to_copy;
        *outbuf += to_copy;
        *outbytesleft -= to_copy;
      }
      if (*inbytesleft > 0) {
        errno = E2BIG;
        return (size_t)-1;
      }
      return 0;
    }

    size_t non_reversible = 0;
    wchar_t wc[2];

    while (*inbytesleft > 0 || d->from_pending_len > 0) {
      // --- Prepare combined input (pending + current) ---
      uint8_t composed[64];
      const uint8_t *cs;
      size_t cs_len;
      bool had_pending;

      if (d->from_pending_len > 0) {
        size_t total = d->from_pending_len + *inbytesleft;
        if (total > sizeof(composed))
          total = sizeof(composed);
        size_t take_from_src = total - d->from_pending_len;
        memcpy(composed, d->from_pending, d->from_pending_len);
        memcpy(composed + d->from_pending_len,
               (const uint8_t *)*inbuf, take_from_src);
        cs = composed;
        cs_len = total;
        had_pending = true;
      } else {
        cs = (const uint8_t *)*inbuf;
        cs_len = *inbytesleft;
        had_pending = false;
      }

      if (cs_len == 0)
        break;

      unsigned int from_cp = d->from_cp;
      int char_len = 0;
      int wide_count = 0;

      // =====================================================
      // DECODE: one source character → wc[2], wide_count
      // =====================================================

      if (is_utf16_family(from_cp)) {
        // UTF-16 source: one code unit (2 bytes) or surrogate pair (4 bytes)
        if (cs_len < 2) {
          // Incomplete code unit — save to pending
          if (!had_pending) {
            memcpy(d->from_pending, cs, cs_len);
            d->from_pending_len = cs_len;
            *inbuf += cs_len;
            *inbytesleft -= cs_len;
          }
          break;
        }

        uint16_t code_unit;
        if (from_cp == 1200)
          code_unit = (uint16_t)cs[0] | ((uint16_t)cs[1] << 8);
        else
          code_unit = ((uint16_t)cs[0] << 8) | (uint16_t)cs[1];

        if (code_unit >= 0xD800 && code_unit <= 0xDBFF) {
          // High surrogate — need 4 bytes total
          char_len = 4;
          if (cs_len < 4) {
            if (!had_pending) {
              size_t to_save = cs_len;
              if (to_save > 4) to_save = 4;
              memcpy(d->from_pending, cs, to_save);
              d->from_pending_len = to_save;
              *inbuf += to_save;
              *inbytesleft -= to_save;
            }
            break;
          }

          uint16_t low;
          if (from_cp == 1200)
            low = (uint16_t)cs[2] | ((uint16_t)cs[3] << 8);
          else
            low = ((uint16_t)cs[2] << 8) | (uint16_t)cs[3];

          if (low >= 0xDC00 && low <= 0xDFFF) {
            wc[0] = code_unit;
            wc[1] = low;
            wide_count = 2;
          } else {
            // Invalid surrogate pair
            if (d->discard_ilseq) {
              non_reversible++;
              advance_input(d, inbuf, inbytesleft, had_pending, 2);
              continue;
            }
            errno = EILSEQ;
            return (size_t)-1;
          }
        } else if (code_unit >= 0xDC00 && code_unit <= 0xDFFF) {
          // Lone low surrogate — invalid
          char_len = 2;
          if (d->discard_ilseq) {
            non_reversible++;
            advance_input(d, inbuf, inbytesleft, had_pending, char_len);
            continue;
          }
          errno = EILSEQ;
          return (size_t)-1;
        } else {
          // BMP character — validate against noncharacters
          // Unicode noncharacters: U+FDD0..U+FDEF, U+FFFE, U+FFFF
          if ((code_unit >= 0xFDD0 && code_unit <= 0xFDEF) ||
              code_unit == 0xFFFE || code_unit == 0xFFFF) {
            char_len = 2;
            if (d->discard_ilseq) {
              non_reversible++;
              advance_input(d, inbuf, inbytesleft, had_pending, char_len);
              continue;
            }
            errno = EILSEQ;
            return (size_t)-1;
          }
          char_len = 2;
          wc[0] = code_unit;
          wide_count = 1;
        }
      }
      else if (is_utf32_family(from_cp)) {
        // UTF-32 source: always 4 bytes per character
        char_len = 4;
        if (cs_len < 4) {
          if (!had_pending) {
            size_t to_save = cs_len;
            if (to_save > 4) to_save = 4;
            memcpy(d->from_pending, cs, to_save);
            d->from_pending_len = to_save;
            *inbuf += to_save;
            *inbytesleft -= to_save;
          }
          break;
        }

        uint32_t codepoint;
        if (from_cp == 12000) {
          codepoint = cs[0] | ((uint32_t)cs[1] << 8) |
                      ((uint32_t)cs[2] << 16) | ((uint32_t)cs[3] << 24);
        } else {
          codepoint = ((uint32_t)cs[0] << 24) | ((uint32_t)cs[1] << 16) |
                      ((uint32_t)cs[2] << 8) | cs[3];
        }

        if (codepoint <= 0xFFFF) {
          wc[0] = (wchar_t)codepoint;
          wide_count = 1;
        } else if (codepoint <= 0x10FFFF) {
          codepoint -= 0x10000;
          wc[0] = 0xD800 + (wchar_t)(codepoint >> 10);
          wc[1] = 0xDC00 + (wchar_t)(codepoint & 0x3FF);
          wide_count = 2;
        } else {
          // Invalid codepoint
          if (d->discard_ilseq) {
            non_reversible++;
            advance_input(d, inbuf, inbytesleft, had_pending, char_len);
            continue;
          }
          errno = EILSEQ;
          return (size_t)-1;
        }
      }
      else {
        // MB source encoding: determine char byte length, then decode
        char_len = source_char_len(from_cp, cs, cs_len);

        if (char_len > (int)cs_len) {
          // Not enough bytes for a complete character
          save_to_pending(d, inbuf, inbytesleft, had_pending,
                          cs, cs_len, char_len);
          break;
        }

        wide_count = MultiByteToWideChar(from_cp, MB_ERR_INVALID_CHARS,
                                          (const char *)cs, char_len,
                                          wc, 2);

        if (wide_count == 0) {
          // Decode error — invalid byte sequence
          if (d->discard_ilseq) {
            non_reversible++;
            advance_input(d, inbuf, inbytesleft, had_pending, char_len);
            continue;
          }
          errno = EILSEQ;
          return (size_t)-1;
        }
      }

      // =====================================================
      // ENCODE: one character → output
      // =====================================================
      int result = encode_one(d, wc, wide_count, outbuf, outbytesleft,
                              &non_reversible);
      if (result < 0) {
        // Don't advance input — next call will re-see the same bytes
        return (size_t)-1;
      }

      // Commit consumed input bytes
      advance_input(d, inbuf, inbytesleft, had_pending, char_len);
    }

    // If the input ended with an incomplete multi-byte sequence, report error.
    // This matches the behavior of the old batch implementation where
    // MultiByteToWideChar(MB_ERR_INVALID_CHARS) rejects incomplete sequences.
    if (d->from_pending_len > 0 && *inbytesleft == 0) {
      d->from_pending_len = 0;
      errno = EILSEQ;
      return (size_t)-1;
    }

    return non_reversible;
  }
} // namespace libiconv
