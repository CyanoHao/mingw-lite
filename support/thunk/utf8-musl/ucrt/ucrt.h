#pragma once

#include <stdio.h>

namespace mingw_thunk
{
  namespace musl_ucrt
  {
    bool is_console(::FILE *fp);
    bool is_console(int fd);
  } // namespace musl_ucrt
} // namespace mingw_thunk
