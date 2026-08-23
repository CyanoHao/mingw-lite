#pragma once

#include <stdlib.h>

namespace mingw_thunk
{
  namespace f
  {
    int win9x__wsystem(const wchar_t *command);
  } // namespace f
} // namespace mingw_thunk
