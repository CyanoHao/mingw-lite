#pragma once

#include <direct.h>

namespace mingw_thunk
{
  namespace f
  {
    int win9x__wrmdir(const wchar_t *path);
  } // namespace f
} // namespace mingw_thunk
