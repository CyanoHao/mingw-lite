#pragma once

#include "util/basic_string_view.h"

namespace libiconv
{
  struct name_entry
  {
    string_view name;
    unsigned int cp;
  };

  extern const name_entry *const name_table_begin;
  extern const name_entry *const name_table_end;
} // namespace libiconv
