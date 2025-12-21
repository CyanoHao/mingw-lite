#include <thunk/_common.h>
#include <thunk/console.h>

#include <stdio.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(msvcrt, 0, int, __cdecl, fputc, int c, FILE *stream)
  {
    int fd = _fileno(stream);
    if (!internal::is_console(fd))
      return get_fputc()(c, stream);

    unsigned char ch = c;
    auto &buffer = internal::fd_io_buffer::get(fd);
    buffer.push(ch);

    if (internal::is_buffered(stream)) {
      if (ch == '\n')
        buffer.flush_all(fd);
    } else {
      buffer.flush_complete_sequence(fd);
    }
    return ch;
  }
} // namespace mingw_thunk
