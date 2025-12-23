#pragma once

#include <io.h>
#include <stdio.h>

#include <consoleapi.h>
#include <minwindef.h>

namespace mingw_thunk::internal
{
  inline bool is_console(HANDLE h) noexcept
  {
    DWORD mode;
    return GetConsoleMode(h, &mode);
  }

  inline bool is_console(int fd) noexcept
  {
    return is_console(_get_osfhandle(fd));
  }

  inline bool is_console(FILE *fp) noexcept
  {
    return is_console(_fileno(fp));
  }
} // namespace mingw_thunk::internal
