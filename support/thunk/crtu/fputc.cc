#include <thunk/_crt.h>
#include <thunk/console.h>
#include <thunk/string.h>
#include <thunk/unicode.h>

#include <stdio.h>

namespace mingw_thunk
{
  __DEFINE_CRT_THUNK(int, fputc, int c, FILE *stream)
  {
    static stl::string buf;
    unsigned char ch = c;
    int fd = _fileno(stream);
    HANDLE fh = (HANDLE)_get_osfhandle(fd);

    if (!internal::is_console(fh)) {
      int res = _write(fd, &ch, 1);
      return res == 1 ? ch : EOF;
    }

    int len = internal::u8_dec_len(ch);
    if (len > 0) {
      // leading byte
      if (!buf.empty()) {
        // previous not finished
        wchar_t rep = internal::u16_rep;
        DWORD written;
        WriteConsoleW(fh, &rep, 1, &written, nullptr);
        buf.clear();
      }
      if (len == 1) {
        // single byte, write to console
        wchar_t wch = ch;
        DWORD written;
        BOOL ok = WriteConsoleW(fh, &wch, 1, &written, nullptr);
        return (ok && written == 1) ? ch : EOF;
      } else {
        // multi byte, push to buffer
        buf.push_back(ch);
        return ch;
      }
    } else {
      // trailing byte
      if (buf.empty()) {
        // unexpected trailing byte
        wchar_t rep = internal::u16_rep;
        DWORD written;
        BOOL ok = WriteConsoleW(fh, &rep, 1, &written, nullptr);
        return (ok && written == 1) ? rep : EOF;
      }
      int expect = internal::u8_dec_len(buf[0]);
      if (buf.size() + 1 < expect) {
        // go on
        buf.push_back(ch);
        return ch;
      } else
        switch (expect) {
        case 2: {
          uint8_t b0 = buf[0];
          uint8_t b1 = ch;
          buf.clear();
          char32_t cp = internal::u8_dec_2({b0, b1});
          wchar_t wch = cp;
          DWORD written;
          BOOL ok = WriteConsoleW(fh, &wch, 1, &written, nullptr);
          return (ok && written == 1) ? ch : EOF;
        }
        case 3: {
          uint8_t b0 = buf[0];
          uint8_t b1 = buf[1];
          uint8_t b2 = ch;
          buf.clear();
          char32_t cp = internal::u8_dec_3({b0, b1, b2});
          wchar_t wch = cp;
          DWORD written;
          BOOL ok = WriteConsoleW(fh, &wch, 1, &written, nullptr);
          return (ok && written == 1) ? ch : EOF;
        }
        case 4: {
          uint8_t b0 = buf[0];
          uint8_t b1 = buf[1];
          uint8_t b2 = buf[2];
          uint8_t b3 = ch;
          buf.clear();
          char32_t cp = internal::u8_dec_4({b0, b1, b2, b3});
          auto u16 = internal::u16_enc_2(cp);
          wchar_t wch[2] = {u16.high, u16.low};
          DWORD written;
          BOOL ok = WriteConsoleW(fh, wch, 2, &written, nullptr);
          return (ok && written == 2) ? ch : EOF;
        }
        default:
          __builtin_unreachable();
        }
    }
  }
} // namespace mingw_thunk
