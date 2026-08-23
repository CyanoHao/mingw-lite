#pragma once

#include <stdio.h>

namespace mingw_thunk
{
  namespace f
  {
    FILE *
    win9x__wfreopen(const wchar_t *path, const wchar_t *mode, FILE *stream);
  } // namespace f
} // namespace mingw_thunk
