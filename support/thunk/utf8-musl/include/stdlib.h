#pragma once

#include <stddef.h>
#include <stdint.h>

#include "../include/features.h"

namespace mingw_thunk
{
  namespace musl
  {
    int wctomb(char *, char32_t);

    hidden int __putenv(char *, size_t, char *);
    hidden void __env_rm_add(char *, char *);
  } // namespace musl
} // namespace mingw_thunk
