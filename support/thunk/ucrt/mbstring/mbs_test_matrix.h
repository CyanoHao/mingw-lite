#pragma once

// plan-3 §5.3 "shared boundary matrix": the byte- and codepoint-edge
// samples every M13 family samples from.  Keeping them in one place is
// what makes "the 105 _mbs* faces and the 34 _ismbc* faces agree about
// what a boundary is" a checkable claim rather than a coincidence.
//
// The codepoint rows are scalar values (plan-3 D7a: the _ismbc*/_mbc*
// argument carries a whole codepoint).  The byte rows are the roles the
// UTF-8 engine assigns; they are the reference DBCS roles with the
// classification predicates swapped (D8a/D8b).

#include <cstddef>

namespace mbsmatrix
{
  // --- codepoint boundary samples -------------------------------------
  struct CpSample
  {
    unsigned int cp;
    const char *what;
  };

  inline constexpr CpSample kCp[] = {
      {0x0000, "NUL"},
      {0x0020, "space"},
      {0x3040, "below hiragana"},
      {0x3041, "hiragana first"},
      {0x3096, "hiragana last"},
      {0x3097, "above hiragana"},
      {0x30A0, "below katakana"},
      {0x30A1, "katakana first"},
      {0x30F6, "katakana last"},
      {0x30F7, "above katakana"},
      {0x33FF, "level 0 last"},
      {0x3400, "level 2 ext-A first"},
      {0x4DBF, "level 2 ext-A last"},
      {0x4E00, "level 1 first"},
      {0x9FA5, "level 1 last"},
      {0x9FA6, "level 2 unified tail"},
      {0xD7FF, "last scalar before surrogates"},
      {0xD800, "high surrogate"},
      {0xDFFF, "low surrogate"},
      {0xE000, "first scalar after surrogates"},
      {0xF900, "level 2 compat first"},
      {0xFAFF, "level 2 compat last"},
      {0x10FFFF, "last scalar"},
      {0x110000, "past the last scalar"},
  };

  // --- byte boundary samples -------------------------------------------
  struct ByteSample
  {
    unsigned char b;
    const char *what;
  };

  inline constexpr ByteSample kByte[] = {
      {0x00, "NUL"},
      {0x41, "ASCII A"},
      {0x7F, "ASCII DEL"},
      {0x80, "continuation first"},
      {0xBF, "continuation last"},
      {0xC0, "overlong lead (never legal)"},
      {0xC1, "overlong lead (never legal)"},
      {0xC2, "lead first"},
      {0xDF, "2-byte lead last"},
      {0xE0, "3-byte lead first"},
      {0xEF, "3-byte lead last"},
      {0xF0, "4-byte lead first"},
      {0xF4, "4-byte lead last"},
      {0xF5, "beyond U+10FFFF"},
      {0xFF, "non-lead high"},
  };

  // --- malformed sequences ----------------------------------------------
  struct BadSeq
  {
    const char *bytes;
    size_t len;
    const char *what;
  };

  inline constexpr BadSeq kBad[] = {
      {"\xC0\x80", 2, "overlong NUL"},
      {"\xE0\x80\x80", 3, "overlong three-byte"},
      {"\xED\xA0\x80", 3, "encoded surrogate"},
      {"\xF4\x90\x80\x80", 4, "past U+10FFFF"},
      {"\xE3\x81", 2, "truncated three-byte"},
      {"\xF0\x9F\x98", 3, "truncated four-byte"},
      {"\x81", 1, "lone continuation"},
      {"\xE3\x81\x41", 3, "continuations then ASCII"},
  };
} // namespace mbsmatrix
