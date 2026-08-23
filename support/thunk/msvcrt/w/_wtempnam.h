#pragma once

#include <stddef.h>

namespace mingw_thunk
{
  namespace f
  {
    wchar_t *win9x__wtempnam(const wchar_t *dir, const wchar_t *prefix);
  } // namespace f
} // namespace mingw_thunk
