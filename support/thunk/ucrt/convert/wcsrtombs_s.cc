#include <thunk/_common.h>
#include <thunk/u8crt/mbstring.h>

#include <errno.h>
#include <stdlib.h>
#include <wchar.h>

namespace mingw_thunk
{
  // Self-authored shell, wine-anchored (see mbsrtowcs_s.cc): success
  // -> *retval = bytes + 1, *src = nullptr; too-small dst -> ERANGE,
  // dst[0] = 0, *retval = 0, *src past the last fully stored code
  // point; EILSEQ (unpaired surrogate) points *src at the start of
  // the offending unit.
  __DEFINE_THUNK(api_ms_win_crt_convert_l1_1_0,
                 0,
                 errno_t,
                 __cdecl,
                 wcsrtombs_s,
                 size_t *retval,
                 char *dst,
                 size_t size,
                 const wchar_t **src,
                 size_t n,
                 mbstate_t *ps)
  {
    using namespace __crt_mbstring;

    static mbstate_t internal = {};
    if (!ps)
      ps = &internal;

    if (!src) {
      if (retval)
        *retval = 0;
      _set_errno(EINVAL);
      return EINVAL;
    }

    if (!dst) {
      if (size != 0) {
        if (retval)
          *retval = 0;
        _set_errno(EINVAL);
        return EINVAL;
      }

      char buf[4];
      size_t total = 0;
      const wchar_t *p = *src;
      size_t left = n;
      while (left) {
        char32_t cp = 0;
        int units = 0;
        wchar_t w1 = *p;
        if (w1 == 0)
          break;
        if (w1 >= 0xd800 && w1 <= 0xdbff) {
          wchar_t w2 = (left > 1) ? p[1] : 0;
          if (w2 >= 0xdc00 && w2 <= 0xdfff) {
            cp = 0x10000 + ((w1 - 0xd800) << 10) + (w2 - 0xdc00);
            units = 2;
          } else {
            units = -1;
          }
        } else if (w1 >= 0xdc00 && w1 <= 0xdfff) {
          units = -1;
        } else {
          cp = w1;
          units = 1;
        }
        if (units < 0) {
          if (retval)
            *retval = 0;
          _set_errno(EILSEQ);
          return EILSEQ;
        }
        total += __c32rtomb_utf8(buf, cp, ps);
        p += units;
        left -= units;
      }
      if (retval)
        *retval = total + 1;
      return 0;
    }

    if (size == 0) {
      if (retval)
        *retval = 0;
      return 0;
    }

    char *out = dst;
    const size_t bytes = size - 1;
    size_t produced = 0;
    char buf[4];
    const wchar_t *p = *src;
    size_t left = n;

    while (left) {
      wchar_t w1 = *p;
      if (w1 == 0)
        break;

      char32_t cp = 0;
      int units = 1;
      if (w1 >= 0xd800 && w1 <= 0xdbff) {
        wchar_t w2 = (left > 1) ? p[1] : 0;
        if (w2 >= 0xdc00 && w2 <= 0xdfff) {
          cp = 0x10000 + ((w1 - 0xd800) << 10) + (w2 - 0xdc00);
          units = 2;
        } else {
          units = -1;
        }
      } else if (w1 >= 0xdc00 && w1 <= 0xdfff) {
        units = -1;
      } else {
        cp = w1;
      }

      if (units < 0) {
        *src = p;
        dst[0] = 0;
        if (retval)
          *retval = 0;
        _set_errno(EILSEQ);
        return EILSEQ;
      }

      size_t r = __c32rtomb_utf8(buf, cp, ps);
      if (produced + r > bytes) {
        *src = p;
        dst[0] = 0;
        if (retval)
          *retval = 0;
        _set_errno(ERANGE);
        return ERANGE;
      }

      for (size_t k = 0; k < r; k++)
        out[k] = buf[k];
      out += r;
      produced += r;
      p += units;
      left -= units;
    }

    const bool complete = (*p == 0);
    *out = 0;
    *src = complete ? nullptr : p;
    if (retval)
      *retval = produced + 1;
    return 0;
  }
} // namespace mingw_thunk
