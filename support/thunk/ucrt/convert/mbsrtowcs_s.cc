#include <thunk/_common.h>
#include <thunk/u8crt/mbstring.h>

#include <errno.h>
#include <stdlib.h>
#include <wchar.h>

namespace mingw_thunk
{
  // Self-authored shell, wine-anchored: success -> *retval = wide chars
  // + 1 (terminator included) and *src = nullptr (complete-string
  // shape); counting mode leaves *src untouched; too-small dst ->
  // ERANGE with dst[0] = 0, *retval = 0 and *src advanced past the
  // last stored character (C11 reading — wine's wcsrtombs_s anchor
  // advanced past one more; noted divergence).  EILSEQ points *src at
  // the start of the invalid sequence (engine parity).
  __DEFINE_THUNK(api_ms_win_crt_convert_l1_1_0,
                 0,
                 errno_t,
                 __cdecl,
                 mbsrtowcs_s,
                 size_t *retval,
                 wchar_t *dst,
                 size_t size,
                 const char **src,
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

    auto readable = [](const char *s) -> size_t {
      if (s[0] == '\0')
        return 1;
      if (s[1] == '\0')
        return 2;
      if (s[2] == '\0')
        return 3;
      return 4;
    };

    if (!dst) {
      if (size != 0) {
        if (retval)
          *retval = 0;
        _set_errno(EINVAL);
        return EINVAL;
      }

      size_t wide = 0;
      const char *p = *src;
      size_t left = n;
      while (left) {
        char32_t c32 = 0;
        size_t avail = readable(p);
        if (avail > left)
          avail = left;
        size_t r = __mbrtoc32_utf8(&c32, p, avail, ps);
        if (r == INVALID) {
          if (retval)
            *retval = 0;
          _set_errno(EILSEQ);
          return EILSEQ;
        }
        if (r == INCOMPLETE || r == 0)
          break;
        p += r;
        left -= r;
        wide += (r == 4) ? 2 : 1;
      }
      if (retval)
        *retval = wide + 1;
      return 0;
    }

    if (size == 0) {
      if (retval)
        *retval = 0;
      return 0;
    }

    wchar_t *out = dst;
    const size_t slots = size - 1;
    size_t produced = 0;
    const char *p = *src;
    size_t left = n;

    while (left) {
      char32_t c32 = 0;
      size_t avail = readable(p);
      if (avail > left)
        avail = left;
      size_t r = __mbrtoc32_utf8(&c32, p, avail, ps);
      if (r == INVALID) {
        *src = p;
        dst[0] = 0;
        if (retval)
          *retval = 0;
        _set_errno(EILSEQ);
        return EILSEQ;
      }
      if (r == INCOMPLETE) {
        // n cut the source mid-sequence: treat as a clean end
        *out = 0;
        *src = p;
        if (retval)
          *retval = produced + 1;
        return 0;
      }
      if (r == 0) {
        *out = 0;
        *src = nullptr; // complete string converted (anchor shape)
        if (retval)
          *retval = produced + 1;
        return 0;
      }

      const size_t units = (r == 4) ? 2 : 1;
      if (produced + units > slots) {
        *src = p;
        dst[0] = 0;
        if (retval)
          *retval = 0;
        _set_errno(ERANGE);
        return ERANGE;
      }

      if (units == 2) {
        c32 -= 0x10000;
        *out++ = static_cast<wchar_t>((c32 >> 10) | 0xd800);
        *out++ = static_cast<wchar_t>((c32 & 0x03ff) | 0xdc00);
        produced += 2;
      } else {
        *out++ = static_cast<wchar_t>(c32);
        produced += 1;
      }
      p += r;
      left -= r;
    }

    *out = 0;
    *src = p; // n exhausted before the terminator
    if (retval)
      *retval = produced + 1;
    return 0;
  }
} // namespace mingw_thunk
