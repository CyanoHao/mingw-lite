#include "../inc/corecrt_internal_mbstring.h"
#include "../inc/corecrt_internal_ptd_propagation.h"

#include <limits.h>

namespace mingw_thunk::ucrt
{

  int __cdecl _wctomb_internal(int *const return_value,
                               char *const destination,
                               size_t const destination_count,
                               wchar_t const wchar,
                               __crt_cached_ptd_host &ptd)
  {
    if (!destination && destination_count > 0)
    {
      if (return_value != nullptr)
        *return_value = 0;

      return 0;
    }

    if (return_value)
      *return_value = -1;

    _UCRT_VALIDATE_RETURN_ERRCODE(ptd, destination_count <= INT_MAX, EINVAL);

    mbstate_t state{};
    int result = static_cast<int>(__crt_mbstring::__c32rtomb_utf8(
        destination, static_cast<char32_t>(wchar), &state, ptd));
    if (return_value != nullptr)
    {
      *return_value = result;
    }
    if (result <= 4)
    {
      return 0;
    }
    else
    {
      return ptd.get_errno().value_or(0);
    }
  }

} // namespace mingw_thunk::ucrt
