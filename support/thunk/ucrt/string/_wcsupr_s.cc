#include <thunk/_common.h>
#include <thunk/u8crt/c_utf8_tables.h>
#include <errno.h>
#include <wchar.h>

namespace mingw_thunk
{
  // Same protocol as the narrow _s pair (wine anchor: rc 22 with
  // reset on too-small, 0 on success)
  __DEFINE_THUNK(api_ms_win_crt_string_l1_1_0, 0, errno_t, __cdecl, _wcsupr_s, wchar_t *str, size_t size)
  {
    if (!str || size == 0) {
      _set_errno(EINVAL);
      return EINVAL;
    }
    size_t length = 0;
    while (length < size && str[length])
      ++length;
    if (length == size) {
      str[0] = 0;
      _set_errno(EINVAL);
      return EINVAL;
    }
    for (size_t k = 0; k < length; k++)
      str[k] = static_cast<wchar_t>(i::u8_wfold_upper(str[k]));
    return 0;
  }
} // namespace mingw_thunk
