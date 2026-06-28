#include "GetUserPreferredUILanguages.h"

#include <thunk/_common.h>
#include <thunk/string.h>

#include <windows.h>
#include <winnls.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32,
                 16,
                 BOOL,
                 WINAPI,
                 GetUserPreferredUILanguages,
                 _In_ DWORD dwFlags,
                 _Out_ PULONG pulNumLanguages,
                 _Out_opt_ PZZWSTR pwszLanguagesBuffer,
                 _Inout_ PULONG pcchLanguagesBuffer)
  {
    __DISPATCH_THUNK_2(GetUserPreferredUILanguages,
                       const auto pfn = try_get_GetUserPreferredUILanguages(),
                       pfn,
                       &f::fallback_GetUserPreferredUILanguages);

    return dllimport_GetUserPreferredUILanguages(
        dwFlags, pulNumLanguages, pwszLanguagesBuffer, pcchLanguagesBuffer);
  }

  namespace f
  {
    BOOL WINAPI
    fallback_GetUserPreferredUILanguages(_In_ DWORD dwFlags,
                                         _Out_ PULONG pulNumLanguages,
                                         _Out_opt_ PZZWSTR pwszLanguagesBuffer,
                                         _Inout_ PULONG pcchLanguagesBuffer)
    {
      LANGID langId = GetUserDefaultUILanguage();

      bool querySize =
          *pcchLanguagesBuffer == 0 && pwszLanguagesBuffer == nullptr;

      if (dwFlags == MUI_LANGUAGE_ID) {
        if (querySize) {
          *pcchLanguagesBuffer = 6;
          *pulNumLanguages = 1;
          return TRUE;
        }
        if (*pcchLanguagesBuffer < 6) {
          *pcchLanguagesBuffer = 6;
          *pulNumLanguages = 0;
          SetLastError(ERROR_INSUFFICIENT_BUFFER);
          return FALSE;
        }
        *pcchLanguagesBuffer = 6;
        *pulNumLanguages = 1;

        wchar_t hex[5];
        unsigned int id = langId;
        for (int i = 3; i >= 0; --i) {
          int d = id & 0x0F;
          hex[i] = d < 10 ? L'0' + d : L'a' + (d - 10);
          id >>= 4;
        }
        hex[4] = L'\0';
        c::wmemcpy(pwszLanguagesBuffer, hex, 5);
        pwszLanguagesBuffer[5] = L'\0';
        return TRUE;
      }

      struct lang_to_locale
      {
        LANGID langid;
        const wchar_t *name;
      };

      static const lang_to_locale k_map[] = {
          {0x0404, L"zh-TW"},
          {0x0409, L"en-US"},
          {0x0416, L"pt-BR"},
          {0x0804, L"zh-CN"},
      };

      const wchar_t *localeName = L"en-US";
      for (size_t i = 0; i < sizeof(k_map) / sizeof(k_map[0]); ++i) {
        if (k_map[i].langid == langId) {
          localeName = k_map[i].name;
          break;
        }
      }

      size_t length = c::wcslen(localeName);
      if (querySize) {
        *pcchLanguagesBuffer = length + 2;
        *pulNumLanguages = 1;
        return TRUE;
      }
      if (*pcchLanguagesBuffer < length + 2) {
        *pcchLanguagesBuffer = length + 2;
        *pulNumLanguages = 0;
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
      }
      *pcchLanguagesBuffer = length + 2;
      *pulNumLanguages = 1;
      c::wcscpy(pwszLanguagesBuffer, localeName);
      pwszLanguagesBuffer[length + 1] = L'\0';
      return TRUE;
    }
  } // namespace f
} // namespace mingw_thunk
