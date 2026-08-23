#include <thunk/_common.h>
#include <thunk/_no_thunk.h>

#include <errno.h>
#include <mbctype.h>

namespace mingw_thunk
{
  // D9: succeeds-but-no-effect.  Native accepts 65001/936/_MB_CP_ANSI/
  // _MB_CP_OEM/_MB_CP_SBCS and any valid code page, returning 0 and
  // switching its DBCS tables (probe F).  The overlay has exactly one
  // multibyte state (UTF-8); accepting the call without any effect
  // avoids a fake state machine that third parties could observe
  // through _mbctype-driven byte semantics.  Invalid values (42) get
  // the native shape: -1 + EINVAL.
  __DEFINE_THUNK(
      api_ms_win_crt_locale_l1_1_0, 0, int, __cdecl, _setmbcp, int code_page)
  {
    if (code_page == _MB_CP_ANSI || code_page == _MB_CP_OEM ||
        code_page == _MB_CP_SBCS ||
        (code_page > 0 && kernel32_IsValidCodePage()(code_page)))
      return 0;

    _set_errno(EINVAL);
    return -1;
  }
} // namespace mingw_thunk
