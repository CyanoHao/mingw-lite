#include <thunk/_common.h>
#include <thunk/u8crt/c_utf8_tables.h>
#include <errno.h>
#include <wchar.h>

namespace mingw_thunk
{
  // In-place wide fold via the towlower/towupper engine (ASCII
  // fast path + >= 0x80 delegate); CJK/surrogates unchanged
  __DEFINE_THUNK(api_ms_win_crt_string_l1_1_0, 0, wchar_t *, __cdecl, _wcslwr, wchar_t *str)
  {
    if (!str) {
      _set_errno(EINVAL);
      return nullptr;
    }
    for (wchar_t *p = str; *p; ++p)
      *p = static_cast<wchar_t>(i::u8_wfold_lower(*p));
    return str;
  }
} // namespace mingw_thunk
