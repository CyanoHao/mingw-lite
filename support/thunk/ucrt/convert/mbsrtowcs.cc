#include <thunk/_common.h>
#include <thunk/u8crt/mbstring.h>

#include <errno.h>
#include <wchar.h>

namespace mingw_thunk
{
  // UCRT __crt_mbstring::__mbsrtowcs_utf8
  __DEFINE_THUNK(api_ms_win_crt_convert_l1_1_0,
                 0,
                 size_t,
                 __cdecl,
                 mbsrtowcs,
                 wchar_t *dst,
                 const char **src,
                 size_t len,
                 mbstate_t *ps)
  {
    using namespace __crt_mbstring;

    // _mbsrtowcs_helper preamble
    if (!src) {
      errno = EINVAL;
      return size_t(-1);
    }
    static mbstate_t internal = {};
    if (ps == nullptr) {
      ps = &internal;
    }

    const char *current_src = *src;

    auto compute_available = [](const char *s) -> size_t {
      // We shouldn't just blindly request to read 4 bytes, because there might
      // not be 4 bytes left to read.
      if (s[0] == '\0') {
        return 1;
      } else if (s[1] == '\0') {
        return 2;
      } else if (s[2] == '\0') {
        return 3;
      }
      return 4;
    };

    if (dst != nullptr) {
      wchar_t *current_dest = dst;
      for (; len > 0; --len) {
        const size_t avail = compute_available(current_src);
        char32_t c32 = 0;
        const size_t retval = __mbrtoc32_utf8(&c32, current_src, avail, ps);
        if (retval == INVALID) {
          // Set src to the beginning of the invalid char
          *src = current_src;
          errno = EILSEQ;
          return retval;
        } else if (retval == 0) {
          current_src = nullptr;
          *current_dest = L'\0';
          break;
        } else if (c32 > 0xffff) {
          // This is going to take two output wchars. Make sure we have enough
          // room for this output.
          if (len > 1) {
            --len;
            c32 -= 0x10000;
            const char16_t high_surrogate =
                static_cast<char16_t>((c32 >> 10) | 0xd800);
            const char16_t low_surrogate =
                static_cast<char16_t>((c32 & 0x03ff) | 0xdc00);
            *current_dest++ = high_surrogate;
            *current_dest++ = low_surrogate;
          } else {
            break;
          }
        } else {
          *current_dest++ = static_cast<wchar_t>(c32);
        }
        current_src += retval;
      }
      *src = current_src;
      return static_cast<size_t>(current_dest - dst);
    } else {
      size_t total_count = 0;
      for (;; ++total_count) {
        const size_t avail = compute_available(current_src);

        const size_t retval = __mbrtoc32_utf8(nullptr, current_src, avail, ps);
        if (retval == __crt_mbstring::INVALID) {
          errno = EILSEQ;
          return retval;
        } else if (retval == 0) {
          break;
        } else if (retval == 4) {
          // SMP characters take two UTF-16 wide chars
          ++total_count;
        } else {
          // This should be impossible. Means we encountered a multibyte char
          // that extended past the null terminator, or is more than 4 bytes
          // long
          /* _ASSERTE(retval != __crt_mbstring::INCOMPLETE); */
        }
        current_src += retval;
      }
      return total_count;
    }
  }
} // namespace mingw_thunk
