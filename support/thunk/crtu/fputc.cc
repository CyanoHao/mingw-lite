#include <thunk/_crt.h>
#include <thunk/console.h>
#include <thunk/string.h>
#include <thunk/unicode.h>

#include <stdio.h>

namespace mingw_thunk
{
  __DEFINE_CRT_THUNK(int, fputc, int c, FILE *stream)
  {
    if (!internal::is_console(stream)) {
      unsigned char ch = c;
      int res = _write(_fileno(stream), &ch, 1);
      return res == 1 ? ch : EOF;
    }

    static stl::string buf;
    #error TODO

    int fd = _fileno(stream);
    HANDLE h = (HANDLE)_get_osfhandle(fd);
  }
} // namespace mingw_thunk
