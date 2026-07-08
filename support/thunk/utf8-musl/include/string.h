#pragma once

#include "../include/features.h"

namespace mingw_thunk
{
  namespace musl
  {
    hidden char *__strchrnul(const char *, int);
  }
} // namespace mingw_thunk
