#include "../internal/stdio_impl.h"
#include <io.h>
#include <thunk/string.h>
#include <thunk/unicode.h>
#include <wchar.h>

namespace mingw_thunk
{
  namespace musl
  {
    namespace
    {
      constexpr size_t MERGE_BUFFER_SIZE = 8192;

      static size_t find_safe_chunk(const unsigned char *data, size_t max_len)
      {
        if (max_len == 0) {
          return 0;
        }

        size_t pos = 0;
        while (pos < max_len) {
          unsigned char lead = data[pos];
          int seq_len = i::u8_dec_len(lead);

          if (seq_len <= 0 || seq_len > 4) {
            pos++;
            continue;
          }

          if (pos + seq_len > max_len) {
            break;
          }

          bool valid = true;
          for (int i = 1; i < seq_len; ++i) {
            if (!i::u8_is_trail(data[pos + i])) {
              valid = false;
              break;
            }
          }

          if (!valid) {
            pos++;
            continue;
          }

          pos += seq_len;
        }

        return pos;
      }

      static size_t safe_chunk_size(const unsigned char *data, size_t remaining)
      {
        size_t max_chunk = remaining > 4096 ? 4096 : remaining;
        return find_safe_chunk(data, max_chunk);
      }

      static size_t write_utf8_data(HANDLE h,
                                    const unsigned char *data,
                                    size_t len,
                                    FILE *f,
                                    d::w_str &wbuf)
      {
        size_t total_written = 0;
        const unsigned char *ptr = data;
        size_t remaining = len;

        while (remaining > 0) {
          size_t chunk_size = safe_chunk_size(ptr, remaining);

          if (chunk_size == 0) {
            break;
          }

          if (!wbuf.from_u((const char *)ptr, (int)chunk_size)) {
            f->flags |= F_ERR;
            return total_written;
          }

          DWORD written;
          if (!WriteConsoleW(
                  h, wbuf.data(), (DWORD)wbuf.size(), &written, nullptr)) {
            f->flags |= F_ERR;
            return total_written;
          }

          if (written < (DWORD)wbuf.size()) {
            f->flags |= F_ERR;
            total_written += chunk_size;
            return total_written;
          }

          total_written += chunk_size;
          ptr += chunk_size;
          remaining -= chunk_size;
        }

        return total_written;
      }
    } // namespace

    hidden size_t __console_write(FILE *f, const unsigned char *buf, size_t len)
    {
      HANDLE h = (HANDLE)_get_osfhandle(f->fd);
      if (h == INVALID_HANDLE_VALUE) {
        f->flags |= F_ERR;
        return 0;
      }

      d::w_str wbuf;
      size_t total_written = 0;
      size_t buffered = f->wpos - f->wbase;

      if (len == 0) {
        if (buffered == 0) {
          return 0;
        }

        size_t written = write_utf8_data(h, f->wbase, buffered, f, wbuf);

        if (written >= buffered) {
          f->wbase = f->wpos = f->buf;
          f->wend = f->buf + f->buf_size;
        } else {
          f->wbase += written;
          f->wpos = f->wbase;
        }
        return written;
      }

      if (buffered > 0) {
        unsigned char temp_buf[MERGE_BUFFER_SIZE];
        size_t use_temp = (buffered + len <= MERGE_BUFFER_SIZE);
        unsigned char *merge_buf =
            use_temp ? temp_buf : (unsigned char *)alloca(buffered + len);

        memcpy(merge_buf, f->wbase, buffered);
        memcpy(merge_buf + buffered, buf, len);

        size_t written = write_utf8_data(h, merge_buf, buffered + len, f, wbuf);

        if (written >= buffered) {
          f->wbase = f->wpos = f->buf;
          f->wend = f->buf + f->buf_size;
          total_written = written;
        } else {
          f->wbase += written;
          f->wpos = f->wbase;
          total_written = written;
        }
      } else {
        size_t written = write_utf8_data(h, buf, len, f, wbuf);

        if (written >= len) {
          f->wbase = f->wpos = f->buf;
          f->wend = f->buf + f->buf_size;
        }
        total_written = written;
      }

      return total_written;
    }
  } // namespace musl
} // namespace mingw_thunk
