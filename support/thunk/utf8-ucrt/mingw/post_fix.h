#pragma once

#include <crtdbg.h>

#include <minwindef.h>
#include <winnt.h>

/* corecrt.h */

#if defined _PREFAST_ && defined _CA_SHOULD_CHECK_RETURN
#define _Check_return_opt_ _Check_return_
#else
#define _Check_return_opt_
#endif

namespace mingw_thunk::ucrt
{

  template <size_t N>
  void __cdecl _invoke_watson(wchar_t const *const expression,
                              char const (&function_name)[N],
                              wchar_t const *const file_name,
                              unsigned int const line_number,
                              uintptr_t const reserved)
  {
    wchar_t w_function_name[N];

    // assume ASCII
    for (size_t i = 0; i < N; i++)
      w_function_name[i] = function_name[i];

    ::_invoke_watson(
        expression, w_function_name, file_name, line_number, reserved);
  }

} // namespace mingw_thunk::ucrt

#define _ASSERT_AND_INVOKE_WATSON(expr)                                        \
  {                                                                            \
    _ASSERTE((expr));                                                          \
    if (!(expr))                                                               \
    {                                                                          \
      mingw_thunk::ucrt::_invoke_watson(                                       \
          L"" #expr, __FUNCTION__, L"" __FILE__, __LINE__, 0);                 \
    }                                                                          \
  }

/* winnt.h */

#undef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(P) ((void)(P))
