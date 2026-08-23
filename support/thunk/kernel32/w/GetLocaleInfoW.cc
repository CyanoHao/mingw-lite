#include "GetLocaleInfoW.h"

#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/os.h>
#include <thunk/string.h>

#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32,
                 16,
                 INT,
                 WINAPI,
                 GetLocaleInfoW,
                 _In_ LCID Locale,
                 _In_ LCTYPE LCType,
                 _Out_opt_ LPWSTR lpLCData,
                 _In_ int cchData)
  {
    __DISPATCH_THUNK_2(GetLocaleInfoW,
                       i::is_nt(),
                       &__ms_GetLocaleInfoW,
                       &f::win9x_GetLocaleInfoW);

    return dllimport_GetLocaleInfoW(Locale, LCType, lpLCData, cchData);
  }

  namespace f
  {
    static UINT get_locale_cp(LCID lcid, LCTYPE lctype) noexcept
    {
      if (lctype & LOCALE_USE_CP_ACP)
        return CP_ACP;

      // LOCALE_RETURN_NUMBER is not supported on Windows 95.
      char buf[16] = {};
      if (!__ms_GetLocaleInfoA(lcid, LOCALE_IDEFAULTANSICODEPAGE, buf, 16))
        return CP_ACP;

      UINT cp = 0;
      for (const char *p = buf; *p >= '0' && *p <= '9'; ++p)
        cp = cp * 10 + (*p - '0');
      return cp;
    }

    INT __stdcall win9x_GetLocaleInfoW(_In_ LCID Locale,
                                       _In_ LCTYPE LCType,
                                       _Out_opt_ LPWSTR lpLCData,
                                       _In_ int cchData)
    {
      if (cchData < 0 || (cchData && !lpLCData)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
      }

      // binary data semantic
      if (LOWORD(LCType) == LOCALE_FONTSIGNATURE ||
          (LCType & LOCALE_RETURN_NUMBER)) {
        const int wide_len = cchData * 2;
        const int ret =
            __ms_GetLocaleInfoA(Locale, LCType, (LPSTR)lpLCData, wide_len);
        return ret / 2;
      }

      const UINT cp = get_locale_cp(Locale, LCType);

      d::cp_str cp_buf{d::max_path_tag{}};
      int a_len = __ms_GetLocaleInfoA(Locale, LCType, cp_buf.data(), MAX_PATH);
      if (a_len == 0)
        return 0;

      while (a_len > cp_buf.size()) {
        if (!cp_buf.resize(a_len)) {
          SetLastError(ERROR_OUTOFMEMORY);
          return 0;
        }
        a_len = __ms_GetLocaleInfoA(Locale, LCType, cp_buf.data(), a_len);
        if (a_len == 0)
          return 0;
      }

      d::w_str w_buf;
      // including final null
      if (!w_buf.from_cp(cp, cp_buf.data(), a_len)) {
        SetLastError(ERROR_OUTOFMEMORY);
        return 0;
      }

      if (cchData == 0)
        return w_buf.size();

      if (w_buf.size() > cchData) {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return 0;
      }

      c::wmemcpy(lpLCData, w_buf.data(), w_buf.size());
      return w_buf.size();
    }
  } // namespace f
} // namespace mingw_thunk
