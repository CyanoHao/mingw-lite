#pragma once

// plan-3 M10 (D5/D10): the overlay has exactly one locale state — C.UTF-8
// — so _create_locale/_wcreate_locale/_get_current_locale all hand out the
// same process-wide dummy __crt_locale_pointers and _free_locale is a
// no-op.  Feeding the sentinel to a native _l-suffixed function is UB
// (known limitation, D5 note).  Defined in _get_current_locale.cc.

#include <locale.h>

namespace mingw_thunk
{
  namespace i
  {
    extern _locale_tstruct u8_locale_sentinel;
  } // namespace i
} // namespace mingw_thunk
