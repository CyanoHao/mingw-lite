#pragma once

#include <locale.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <wctype.h>

#include <sal.h>

namespace mingw_thunk::ucrt
{

#define _CRT_UNBOUNDED_BUFFER_SIZE (static_cast<size_t>(-1))
#define _Maybe_unsafe_(buffer_annotation, expr)                                \
  _When_((expr < _CRT_UNBOUNDED_BUFFER_SIZE), buffer_annotation(expr))         \
      _When_((expr >= _CRT_UNBOUNDED_BUFFER_SIZE),                             \
             buffer_annotation(_Inexpressible_("unsafe")))

  _Ret_z_ _Success_(return != 0) char **__acrt_capture_narrow_argv(
      _In_ va_list *arglist,
      _In_z_ char const *first_argument,
      _When_(return == caller_array, _Post_z_)
          _Out_writes_(caller_array_count) char **caller_array,
      _In_ size_t caller_array_count);

  _Ret_z_ _Success_(return != 0) wchar_t **__acrt_capture_wide_argv(
      _In_ va_list *arglist,
      _In_z_ wchar_t const *first_argument,
      _When_(return == caller_array, _Post_z_) _Out_writes_(caller_array_count)
          wchar_t **caller_array,
      _In_ size_t caller_array_count);

#define _CORECRT_GENERATE_FORWARDER(prefix, callconv, name, callee_name)       \
  template <typename... Params>                                                \
  prefix auto callconv name(Params &&...args) throw()                          \
      -> decltype(callee_name(args...))                                        \
  {                                                                            \
    return callee_name(args...);                                               \
  }

  _Check_return_ __forceinline unsigned char __cdecl
  _toupper_fast_internal(_In_ unsigned char const c)
  {
    return c >= 0x80 ? c : towupper(c);
  }

  _Check_return_ __forceinline unsigned char __cdecl
  _tolower_fast_internal(_In_ unsigned char const c)
  {
    return c >= 0x80 ? c : towlower(c);
  }

  _Check_return_ __forceinline unsigned short __cdecl
  _ctype_fast_check_internal(_In_ unsigned char const c, _In_ int const _Mask)
  {
    return c >= 0x80 ? 0 : iswctype(c, _Mask);
  }

  _Check_return_ __forceinline unsigned short __cdecl
  _isdigit_fast_internal(_In_ unsigned char const c)
  {
    return _ctype_fast_check_internal(c, _DIGIT);
  }

  class __crt_cached_ptd_host;

  class _LocaleUpdate
  {
  public:
    explicit _LocaleUpdate(_locale_t const locale) throw()
    {
    }

    _locale_t GetLocaleT() throw()
    {
      return g_locale_c();
    }

  public:
    static _locale_t g_locale_c() noexcept
    {
      static _locale_t const loc = _wcreate_locale(LC_ALL, L"C");
      return loc;
    }
  };

#define _ERRCHECK(e) (e)
#define _ERRCHECK_EINVAL_ERANGE(e) (e)

#define _SECURECRT__FILL_STRING(_String, _Count, _Offset)

} // namespace mingw_thunk::ucrt
