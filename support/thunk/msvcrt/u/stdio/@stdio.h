#pragma once

#include <io.h>
#include <stdio.h>

#include <windows.h>

namespace mingw_thunk
{
  namespace i
  {
    inline bool is_console(HANDLE fh) noexcept
    {
      DWORD mode;
      return GetConsoleMode(fh, &mode);
    }

    inline bool is_console(int fd) noexcept
    {
      intptr_t h = _get_osfhandle(fd);
      return h != -1 && is_console((HANDLE)h);
    }

    inline bool is_console(FILE *fp) noexcept
    {
      return is_console(_fileno(fp));
    }
  } // namespace i
} // namespace mingw_thunk
