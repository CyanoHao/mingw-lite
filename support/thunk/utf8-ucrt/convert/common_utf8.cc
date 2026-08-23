#include "../inc/corecrt_internal_mbstring.h"
#include "../inc/corecrt_internal_ptd_propagation.h"

#include <stddef.h>

namespace mingw_thunk::ucrt::__crt_mbstring
{

  size_t return_illegal_sequence(mbstate_t *ps, __crt_cached_ptd_host &ptd)
  {
    *ps = {};
    ptd.get_errno().set(EILSEQ);
    return INVALID;
  }

  size_t reset_and_return(size_t retval, mbstate_t *ps)
  {
    *ps = {};
    return retval;
  }

} // namespace mingw_thunk::ucrt::__crt_mbstring
