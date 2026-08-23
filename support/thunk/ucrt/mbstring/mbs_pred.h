#pragma once

// plan-3 §5.2 M13a: the wiring between the 34 _ismbc* / 28 _ismbb* /
// _ismbs* / _mbbtype / _mbsbtype faces and the UTF-8 engine in
// include/thunk/u8crt/utf8_mbs.h.  One dispatch per predicate family so
// each face file is a one-line thunk; the decision record for every
// choice lives in plan-3 §5.0.2 (wine anchoring) and §5.0.3 (D7/D8).
//
// Two conventions the whole family follows:
//   * the _l forms ignore the locale (api-set global principle 2),
//   * every predicate returns 0 or 1 (plan-3 D8e: native returns an
//     un-ABI'd ctype mask, 0/1 keeps the family assertable).

#include <thunk/u8crt/utf8_mbs.h>

#include <errno.h>
#include <locale.h>
#include <stddef.h>

namespace mingw_thunk
{
  namespace mbstring
  {
    // _ismbc* — the unsigned int argument carries a whole scalar value
    // (plan-3 D7a).  Below U+0100 the two static tables answer; above it
    // the OS CT_CTYPE1 classification does.
    enum : int
    {
      P_ALNUM,
      P_ALPHA,
      P_BLANK,
      P_DIGIT,
      P_GRAPH,
      P_HIRA,
      P_KATA,
      P_L0,
      P_L1,
      P_L2,
      P_LEGAL,
      P_LOWER,
      P_PRINT,
      P_PUNCT,
      P_SPACE,
      P_SYMBOL,
      P_UPPER
    };

    inline int ismbc(unsigned int c, int pred) noexcept
    {
      switch (pred) {
      case P_ALNUM:
        return __crt_mbs::is_alnum_cp(c) ? 1 : 0;
      case P_ALPHA:
        return __crt_mbs::is_alpha_cp(c) ? 1 : 0;
      case P_BLANK:
        return __crt_mbs::is_blank_cp(c) ? 1 : 0;
      case P_DIGIT:
        return __crt_mbs::is_digit_cp(c) ? 1 : 0;
      case P_GRAPH:
        return __crt_mbs::is_graph_cp(c) ? 1 : 0;
      case P_HIRA:
        return __crt_mbs::scalar_valid(c) && __crt_mbs::is_hiragana(c) ? 1 : 0;
      case P_KATA:
        return __crt_mbs::scalar_valid(c) && __crt_mbs::is_katakana(c) ? 1 : 0;
      case P_L0:
        return __crt_mbs::scalar_valid(c) && __crt_mbs::is_mb_level0(c) ? 1 : 0;
      case P_L1:
        return __crt_mbs::scalar_valid(c) && __crt_mbs::is_mb_level1(c) ? 1 : 0;
      case P_L2:
        return __crt_mbs::scalar_valid(c) && __crt_mbs::is_mb_level2(c) ? 1 : 0;
      case P_LEGAL:
        return __crt_mbs::scalar_valid(c) ? 1 : 0;
      case P_LOWER:
        return __crt_mbs::is_lower_cp(c) ? 1 : 0;
      case P_PRINT:
        return __crt_mbs::is_print_cp(c) ? 1 : 0;
      case P_PUNCT:
        return __crt_mbs::is_punct_cp(c) ? 1 : 0;
      case P_SPACE:
        return __crt_mbs::is_space_cp(c) ? 1 : 0;
      case P_SYMBOL:
        return __crt_mbs::scalar_valid(c) && __crt_mbs::is_mb_symbol(c) ? 1 : 0;
      case P_UPPER:
        return __crt_mbs::is_upper_cp(c) ? 1 : 0;
      default:
        return 0;
      }
    }

    // _ismbb* — a single byte's role.  Anything at or above 0x80 is
    // neither ASCII nor kana, so the six ctype predicates go false and
    // the four kana predicates go false unconditionally (plan-3 D8i:
    // UTF-8 has no single-byte katakana, so this is the correct answer,
    // not a stub).
    enum : int
    {
      B_ALNUM,
      B_ALPHA,
      B_BLANK,
      B_GRAPH,
      B_KALNUM,
      B_KANA,
      B_KPRINT,
      B_KPUNCT,
      B_LEAD,
      B_PRINT,
      B_PUNCT,
      B_TRAIL
    };

    inline int ismbb(unsigned int c, int pred) noexcept
    {
      if (c > 0xFF)
        return 0;

      const unsigned char b = (unsigned char)c;
      switch (pred) {
      case B_LEAD:
        return __crt_mbs::lead_byte(b) ? 1 : 0;
      case B_TRAIL:
        return __crt_mbs::trail_byte(b) ? 1 : 0;
      default:
        break;
      }

      if (b >= 0x80)
        return 0; // not ASCII, and never single-byte kana
      const unsigned short t = i::u8_pctype[b];
      switch (pred) {
      case B_ALNUM:
        return (t & (i::M_UPPER | i::M_LOWER | i::M_DIGIT)) ? 1 : 0;
      case B_ALPHA:
        return (t & (i::M_UPPER | i::M_LOWER)) ? 1 : 0;
      case B_BLANK:
        return (t & i::M_BLANK) ? 1 : 0;
      case B_GRAPH:
        return ((t & (i::M_UPPER | i::M_LOWER | i::M_DIGIT | i::M_PUNCT |
                      i::M_BLANK)) &&
                !(t & (i::M_SPACE | i::M_CONTROL)))
                   ? 1
                   : 0;
      case B_PRINT:
        return (t & i::M_CONTROL) == 0 ? 1 : 0;
      case B_PUNCT:
        return (t & i::M_PUNCT) ? 1 : 0;
      default:
        return 0; // B_KALNUM / B_KANA / B_KPRINT / B_KPUNCT
      }
    }

    // _mbsbtype: a null string, or an offset at or past the terminator,
    // is _MBC_ILLEGAL with errno EINVAL — the reference shell's
    // `_VALIDATE_RETURN(*string != '\0', EINVAL, _MBC_ILLEGAL)`.  The
    // offset is otherwise answered by the engine's UTF-8 walk.
    inline int mbsbtype(const unsigned char *s, size_t pos) noexcept
    {
      using __crt_mbs::MBC_ILLEGAL;
      using __crt_mbs::walk_to;

      if (!s) {
        _set_errno(EINVAL);
        return MBC_ILLEGAL;
      }

      const unsigned char *p = walk_to(s, s + pos);
      if (!p || *p == '\0') {
        _set_errno(EINVAL);
        return MBC_ILLEGAL;
      }
      return __crt_mbs::string_byte_type(s, pos);
    }

    // _ismbslead / _ismbstrail.  A null or out-of-range pointer pair
    // answers 0 and leaves errno alone (wine anchor: the reference's
    // invalid-parameter path returns 0 with errno still 0).
    inline int ismbslead(const unsigned char *s,
                         const unsigned char *current) noexcept
    {
      return __crt_mbs::ptr_is_lead(s, current) ? 1 : 0;
    }

    inline int ismbstrail(const unsigned char *s,
                          const unsigned char *current) noexcept
    {
      return __crt_mbs::ptr_is_trail(s, current) ? 1 : 0;
    }
  } // namespace mbstring
} // namespace mingw_thunk
