#pragma once

#include "config.h"

#include "../iterator.h"
#include "../type_traits.h"

namespace NS_NOSTL
{
  namespace internal
  {
    template <typename IC>
    struct is_contiguous_iterator : is_same<IC, contiguous_iterator_tag> {};

    template <typename IC>
    constexpr bool is_contiguous_iterator_v = is_contiguous_iterator<IC>::value;
  } // namespace internal
} // namespace NS_NOSTL
