#include <thunk/_common.h>
#include <thunk/u8crt/mbstring.h>

#include <limits.h>
#include <string.h>
#include <wchar.h>

namespace mingw_thunk
{
  // UCRT __crt_mbstring::__wcsrtombs_utf8
  __DEFINE_THUNK(api_ms_win_crt_convert_l1_1_0,
                 0,
                 size_t,
                 __cdecl,
                 wcsrtombs,
                 char *dst,
                 const wchar_t **src,
                 size_t len,
                 mbstate_t *ps)
  {
    using namespace __crt_mbstring;

    const wchar_t *current_src = *src;
    char buf[MB_LEN_MAX];

    if (dst != nullptr) {
      char *current_dest = dst;

      // Wide chars are actually UTF-16, so a code point might take 2 input
      // units (a surrogate pair) In case of a failure, keep track of where the
      // current code point began, which might be the previous wchar for a
      // surrogate pair
      const wchar_t *start_of_code_point = current_src;
      for (;;) {
        // If we don't have at least 4 MB_CUR_LEN bytes available in the buffer
        // the next char isn't guaranteed to fit, so put it into a temp buffer
        char *temp;
        if (len < 4) {
          temp = buf;
        } else {
          temp = current_dest;
        }
        const size_t retval = __c16rtomb_utf8(temp, *current_src, ps);

        if (retval == __crt_mbstring::INVALID) {
          // Set src to the beginning of the invalid char
          // If this was the second half of a surrogate pair, return the
          // beginning of the surrogate pair
          *src = start_of_code_point;
          return retval;
        }

        if (temp == current_dest) {
          // We wrote in-place. Nothing to do.
        } else if (len < retval) {
          // Won't fit, so bail out
          // If this was the second half of a surrogate pair, make sure we
          // return that location
          current_src = start_of_code_point;
          break;
        } else {
          // Will fit in remaining buffer, so let's copy it over
          memcpy(current_dest, temp, retval);
        }

        if (retval > 0 && current_dest[retval - 1] == '\0') {
          // Reached null terminator, so break out, but don't count that last
          // terminating byte
          current_src = nullptr;
          current_dest += retval - 1;
          break;
        }

        ++current_src;
        if (retval > 0) {
          start_of_code_point = current_src;
        }

        len -= retval;
        current_dest += retval;
      }
      *src = current_src;
      return current_dest - dst;
    } else {
      size_t total_count = 0;
      for (;;) {
        const size_t retval = __c16rtomb_utf8(buf, *current_src, ps);
        if (retval == __crt_mbstring::INVALID) {
          return retval;
        } else if (retval > 0 && buf[retval - 1] == '\0') {
          // Hit null terminator. Don't count it in the return value.
          total_count += retval - 1;
          break;
        }
        total_count += retval;
        ++current_src;
      }
      return total_count;
    }
  }
} // namespace mingw_thunk
