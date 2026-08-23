#include <thunk/_common.h>

#include <errno.h>
#include <stdlib.h>

namespace mingw_thunk
{
  // reference/ucrt/filesystem/makepath.cpp, UTF-8 edition.  The
  // reference's only multibyte touchpoint is _mbsdec when checking
  // whether the directory already ends in a separator; in UTF-8 every
  // continuation byte is >= 0x80, so a '/' or '\\' byte can only be a
  // real separator and the previous BYTE is the previous character
  // (self-synchronizing argument — plan-3 §2.3).
  //
  // wine anchors (probe E + r11): exact-fit capacity still fails (the
  // NUL write is bounds-checked last), failures clear buffer[0] and
  // return/set ERANGE; null buffer or zero count -> EINVAL with the
  // buffer untouched; drive contributes its first character + ':'.
  __DEFINE_THUNK(api_ms_win_crt_filesystem_l1_1_0,
                 0,
                 errno_t,
                 __cdecl,
                 _makepath_s,
                 char *result_buffer,
                 size_t result_count,
                 const char *drive,
                 const char *directory,
                 const char *file_name,
                 const char *extension)
  {
    if (!result_buffer || result_count == 0) {
      _set_errno(EINVAL);
      return EINVAL;
    }

    const bool bounded = result_count != static_cast<size_t>(-1);
    char *out = result_buffer;
    const char *end = bounded ? result_buffer + result_count : nullptr;

    auto out_of_room = [&]() -> errno_t {
      result_buffer[0] = '\0';
      _set_errno(ERANGE);
      return ERANGE;
    };

    if (drive && drive[0] != '\0') {
      if (end && end - out < 2)
        return out_of_room();
      *out++ = drive[0];
      *out++ = ':';
    }

    if (directory && directory[0] != '\0') {
      const char *in = directory;
      while (*in != '\0') {
        if (end && out >= end)
          return out_of_room();
        *out++ = *in++;
      }
      // Trailing separator?  (plain byte test — see the header comment)
      if (*(out - 1) != '/' && *(out - 1) != '\\') {
        if (end && out >= end)
          return out_of_room();
        *out++ = '\\';
      }
    }

    if (file_name) {
      const char *in = file_name;
      while (*in != '\0') {
        if (end && out >= end)
          return out_of_room();
        *out++ = *in++;
      }
    }

    if (extension) {
      if (extension[0] != '\0' && extension[0] != '.') {
        if (end && out >= end)
          return out_of_room();
        *out++ = '.';
      }
      const char *in = extension;
      while (*in != '\0') {
        if (end && out >= end)
          return out_of_room();
        *out++ = *in++;
      }
    }

    if (end && out >= end)
      return out_of_room();

    *out = '\0';
    return 0;
  }
} // namespace mingw_thunk
