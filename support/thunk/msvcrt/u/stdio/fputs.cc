#include <thunk/_common.h>
#include <thunk/console.h>

#include <stdio.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(msvcrt, 0, int, __cdecl, fputs, const char *s, FILE *stream)
  {
    int fd = _fileno(stream);
    if (!internal::is_console(fd))
      return get_fputs()(s, stream);

    size_t len = strlen(s);
    auto &buffer = internal::fd_io_buffer::get(fd);
    buffer.append(s, len);

    if (internal::is_buffered(stream)) {
      buffer.flush_complete_line(fd);
    } else {
      buffer.flush_complete_sequence(fd);
    }
    return len;
  }
} // namespace mingw_thunk
