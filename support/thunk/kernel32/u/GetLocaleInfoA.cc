#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/string.h>

#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32,
                 16,
                 INT,
                 WINAPI,
                 GetLocaleInfoA,
                 _In_ LCID Locale,
                 _In_ LCTYPE LCType,
                 _Out_opt_ LPSTR lpLCData,
                 _In_ int cchData)
  {
    // binary data sematic
    if (LOWORD(LCType) == LOCALE_FONTSIGNATURE ||
        (LCType & LOCALE_RETURN_NUMBER))
      return __ms_GetLocaleInfoA(Locale, LCType, lpLCData, cchData);

    // not ACP sematic
    if (!(LCType & LOCALE_USE_CP_ACP))
      return __ms_GetLocaleInfoA(Locale, LCType, lpLCData, cchData);

    if (cchData < 0 || (cchData && !lpLCData)) {
      SetLastError(ERROR_INVALID_PARAMETER);
      return 0;
    }

    d::w_str w_buf{d::max_path_tag{}};
    int w_len = GetLocaleInfoW(Locale, LCType, w_buf.data(), MAX_PATH);
    if (w_len == 0)
      return 0;

    while (w_len > w_buf.size()) {
      if (!w_buf.resize(w_len)) {
        SetLastError(ERROR_OUTOFMEMORY);
        return 0;
      }
      w_len = GetLocaleInfoW(Locale, LCType, w_buf.data(), w_len);
      if (w_len == 0)
        return 0;
    }

    d::u_str u_buf;
    // including final null
    if (!u_buf.from_w(w_buf.data(), w_len))
      return 0;

    if (cchData == 0)
      return u_buf.size();

    if (u_buf.size() > cchData) {
      SetLastError(ERROR_INSUFFICIENT_BUFFER);
      return 0;
    }

    c::memcpy(lpLCData, u_buf.c_str(), u_buf.size());
    return u_buf.size();
  }
} // namespace mingw_thunk
