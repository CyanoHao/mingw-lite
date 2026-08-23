#pragma once

#include <io.h>

namespace mingw_thunk
{
  namespace f
  {
    int win9x__waccess(const wchar_t *path, int mode);
  } // namespace f
} // namespace mingw_thunk
