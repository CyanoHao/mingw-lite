#pragma once

#include <io.h>

namespace mingw_thunk
{
  namespace f
  {
    int win9x__wsopen(const wchar_t *path, int oflag, int shflag, ...);
  } // namespace f
} // namespace mingw_thunk
