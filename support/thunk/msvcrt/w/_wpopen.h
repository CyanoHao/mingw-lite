#pragma once

#include <stdio.h>

namespace mingw_thunk
{
  namespace f
  {
    FILE *win9x__wpopen(const wchar_t *command, const wchar_t *mode);
  } // namespace f
} // namespace mingw_thunk
