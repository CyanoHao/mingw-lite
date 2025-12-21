#include <processenv.h>
#include <thunk/_crt.h>
#include <thunk/console.h>

#include <stdio.h>

namespace mingw_thunk
{
  __DEFINE_VCRT_THUNK(int, fputs, const char *s, FILE *stream)
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
