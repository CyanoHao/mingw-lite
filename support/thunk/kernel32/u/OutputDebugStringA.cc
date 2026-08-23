#include <thunk/_common.h>
#include <thunk/_no_thunk.h>
#include <thunk/string.h>

#include <windows.h>

namespace mingw_thunk
{
  __DEFINE_THUNK(kernel32,
                 4,
                 VOID,
                 WINAPI,
                 OutputDebugStringA,
                 _In_opt_ LPCSTR lpOutputString)
  {
    if (!lpOutputString)
      return;

    d::w_str w_buf;
    if (!w_buf.from_u(lpOutputString)) {
      SetLastError(ERROR_OUTOFMEMORY);
      return;
    }

    OutputDebugStringW(w_buf.c_str());
  }
} // namespace mingw_thunk
