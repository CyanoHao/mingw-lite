#include <thunk/_common.h>

#include <errno.h>
#include <io.h>

#include <windows.h>

namespace mingw_thunk
{
  // reference/ucrt/lowio/mktemp.cpp shape, UTF-8 edition (see
  // _mktemp.cc).  wine anchors: size 0 / null template -> EINVAL with
  // the template UNTOUCHED; unterminated-in-size / fewer than six
  // trailing X's -> EINVAL with template[0] reset to 0; exhaustion of
  // the probe letters -> EEXIST with template[0] reset.  Errno is
  // restored on success per the reference source (wine leaks ENOENT
  // from the existence probe — noted divergence).
  __DEFINE_THUNK(api_ms_win_crt_stdio_l1_1_0,
                 0,
                 errno_t,
                 __cdecl,
                 _mktemp_s,
                 char *template_string,
                 size_t buffer_size)
  {
    if (!template_string || buffer_size == 0) {
      _set_errno(EINVAL);
      return EINVAL;
    }

    size_t length = 0;
    while (length < buffer_size && template_string[length])
      ++length;
    if (length == buffer_size) { // not null-terminated within size
      template_string[0] = 0;
      _set_errno(EINVAL);
      return EINVAL;
    }
    if (length < 6) {
      template_string[0] = 0;
      _set_errno(EINVAL);
      return EINVAL;
    }

    unsigned number = GetCurrentThreadId();

    char *string = template_string + length;
    size_t replaced = 0;
    while (--string >= template_string && *string == 'X' &&
           replaced < 5) {
      ++replaced;
      *string = static_cast<char>((number % 10) + '0');
      number /= 10;
    }

    if (*string != 'X' || replaced < 5) {
      template_string[0] = 0;
      _set_errno(EINVAL);
      return EINVAL;
    }

    *string = 'a';
    char letter = 'b';

    int saved_errno = errno;
    errno = 0;

    while (_access(template_string, 0) == 0) {
      if (letter > 'z') {
        template_string[0] = 0;
        errno = EEXIST;
        return EEXIST;
      }
      *string = letter++;
      errno = 0;
    }

    errno = saved_errno;
    return 0;
  }
} // namespace mingw_thunk
