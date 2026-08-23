#include "utf8_buffer.h"

#include "console_channel.h"

namespace mingw_thunk
{
  namespace musl
  {
    namespace
    {
      utf8_buffer g_trio[3];

      /* shared fallback when no channel owns the fd (pool exhausted or
       * fd without a handle): a known degraded mode, see plan M1.7 */
      utf8_buffer g_dummy;
    } // namespace

    utf8_buffer_map g_utf8_buffer;

    utf8_buffer &utf8_buffer_map::operator[](int fd) noexcept
    {
      if (fd >= 0 && fd < 3)
        return g_trio[fd];
      if (utf8_buffer *tail = console_channel_tail(fd))
        return *tail;
      return g_dummy;
    }
  } // namespace musl
} // namespace mingw_thunk
