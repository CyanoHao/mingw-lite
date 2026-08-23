#pragma once

#include <stdio.h>

#include <io.h>
#include <windows.h>

namespace mingw_thunk::ucrt
{

  inline bool is_console(int fd)
  {
    if (!_isatty(fd))
      return false;

    HANDLE h = (HANDLE)_get_osfhandle(fd);
    DWORD mode;
    return h != INVALID_HANDLE_VALUE && GetConsoleMode(h, &mode);
  }

  inline bool is_console(FILE *stream)
  {
    return is_console(_fileno(stream));
  }

} // namespace mingw_thunk::ucrt
