#pragma once

#include <io.h>
#include <stdio.h>

#include <windows.h>

namespace mingw_thunk
{
  namespace musl::ucrt
  {
    bool is_console(HANDLE fh) noexcept
    {
      DWORD mode;
      return GetConsoleMode(fh, &mode);
    }

    bool is_console(int fd) noexcept
    {
      HANDLE h = (HANDLE)_get_osfhandle(fd);
      return h != INVALID_HANDLE_VALUE && is_console(h);
    }

    bool is_console(::FILE *fp) noexcept
    {
      return is_console(_fileno(fp));
    }
  } // namespace musl::ucrt
} // namespace mingw_thunk
