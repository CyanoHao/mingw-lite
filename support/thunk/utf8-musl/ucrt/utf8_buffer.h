#pragma once

namespace mingw_thunk
{
  namespace musl
  {
    struct utf8_buffer
    {
      unsigned char buf[4];
      int len;
    };

    /* fd-keyed view over the per-channel tails: fd 0/1/2 map to the
     * static trio buffers, fd >= 3 to the tail of the console channel
     * owning that fd (utf8-musl/win32/console_channel.*).  Keeps the
     * g_utf8_buffer[fd] syntax used by the ported read()/write() so
     * those files stay diff-free. */
    struct utf8_buffer_map
    {
      utf8_buffer &operator[](int fd) noexcept;
    };

    extern utf8_buffer_map g_utf8_buffer;
  } // namespace musl
} // namespace mingw_thunk
