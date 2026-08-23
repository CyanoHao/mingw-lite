#include "OutputDebugStringW.h"

#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/os.h>
#include <thunk/string.h>

#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32,
                 4,
                 VOID,
                 WINAPI,
                 OutputDebugStringW,
                 _In_opt_ LPCWSTR lpOutputString)
  {
    __DISPATCH_THUNK_2(OutputDebugStringW,
                       i::is_nt(),
                       &__ms_OutputDebugStringW,
                       &f::win9x_OutputDebugStringW);

    dllimport_OutputDebugStringW(lpOutputString);
  }

  namespace f
  {
    VOID __stdcall win9x_OutputDebugStringW(_In_opt_ LPCWSTR lpOutputString)
    {
      if (!lpOutputString)
        return;

      d::a_str a_buf;
      if (!a_buf.from_w(lpOutputString)) {
        SetLastError(ERROR_OUTOFMEMORY);
        return;
      }

      __ms_OutputDebugStringA(a_buf.c_str());
    }
  } // namespace f
} // namespace mingw_thunk
