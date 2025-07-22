#pragma once

#include "internal/config.h"

namespace NS_NOCRT
{
  inline int errno = 0;
} // namespace NS_NOCRT

#ifdef ENOMEM
static_assert(ENOMEM == 12, "ENOMEM != 12");
#else
#define ENOMEM 12
#endif

#ifdef EINVAL
static_assert(EINVAL == 22, "EINVAL != 22");
#else
#define EINVAL 22
#endif
