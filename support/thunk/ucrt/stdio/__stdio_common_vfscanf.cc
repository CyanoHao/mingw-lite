#include <errno.h>
#include <stdio.h>
#include <thunk/_common.h>
#include <thunk/_no_thunk.h>

#include <io.h>
#include <windows.h>

#include "@stdio_file.h"

namespace mingw_thunk
{

  namespace
  {
    inline bool is_console(int fd) noexcept
    {
      if (fd < 0 || fd > 2)
        return false;
      if (!_isatty(fd))
        return false;
      HANDLE hConsole = (HANDLE)_get_osfhandle(fd);
      DWORD mode;
      return GetConsoleMode(hConsole, &mode);
    }
  } // namespace
  extern "C" int musl_vfscanf(musl_FILE *f, const char *fmt, va_list ap);

  __DEFINE_THUNK(api_ms_win_crt_stdio_l1_1_0,
                 0,
                 int,
                 __cdecl,
                 __stdio_common_vfscanf,
                 uint64_t options,
                 FILE *stream,
                 const char *format,
                 _locale_t locale,
                 va_list arglist)
  {
    (void)options;
    (void)locale;

    int fd = _fileno(stream);

    if (fd == 0 && is_console(fd)) {
      return musl_vfscanf(g::g_musl_files[0], format, arglist);
    }

    return __ms___stdio_common_vfscanf(
        options, stream, format, locale, arglist);
  }
} // namespace mingw_thunk
