#pragma once

#include "eastl.h"

namespace intl
{
  using string = eastl::basic_string<char, eastl::allocator_malloc>;

  extern string default_domain;
} // namespace intl
