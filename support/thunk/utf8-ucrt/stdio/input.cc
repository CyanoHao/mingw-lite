#include "../inc/corecrt_internal_stdio_input.h"

#include "../mingw/console.h"
#include "../mingw/thunk.h"

namespace mingw_thunk::ucrt
{

  using namespace __crt_stdio_input;

  template <typename Character>
  static int __cdecl common_vfscanf(unsigned __int64 const options,
                                    FILE *const stream,
                                    Character const *const format,
                                    _locale_t const locale,
                                    va_list const arglist) throw()
  {
    typedef input_processor<Character, stream_input_adapter<Character>>
        processor_type;

    _VALIDATE_RETURN(stream != nullptr, EINVAL, EOF);
    _VALIDATE_RETURN(format != nullptr, EINVAL, EOF);

    return __acrt_lock_stream_and_call(
        stream,
        [&]()
        {
          _LocaleUpdate locale_update(locale);

          /* classic CRT doctrine: a read from console stdin flushes
           * stdout/stderr first, so prompts sitting in the UTF-8
           * console channels become visible before the input wait */
          if (_fileno(stream) == 0 && is_console(0))
          {
            if (console *const out = console::get(1, false))
              out->flush_stdout_or_stderr();
            if (console *const err = console::get(2, false))
              err->flush_stdout_or_stderr();
          }

          processor_type processor(stream_input_adapter<Character>(stream),
                                   options,
                                   format,
                                   locale_update.GetLocaleT(),
                                   arglist);

          return processor.process();
        });
  }

  extern "C" int __cdecl __stdio_common_vfscanf(unsigned __int64 const options,
                                                FILE *const stream,
                                                char const *const format,
                                                _locale_t const locale,
                                                va_list const arglist)
  {
    return common_vfscanf(options, stream, format, locale, arglist);
  }

  extern "C" int __cdecl __stdio_common_vfwscanf(unsigned __int64 const options,
                                                 FILE *const stream,
                                                 wchar_t const *const format,
                                                 _locale_t const locale,
                                                 va_list const arglist)
  {
    return common_vfscanf(options, stream, format, locale, arglist);
  }

  template <typename Character>
  static int __cdecl common_vsscanf(unsigned __int64 const options,
                                    Character const *const buffer,
                                    size_t const buffer_count,
                                    Character const *const format,
                                    _locale_t const locale,
                                    va_list const arglist) throw()
  {
    typedef __acrt_stdio_char_traits<Character> char_traits;

    typedef input_processor<Character, string_input_adapter<Character>>
        processor_type;

    _VALIDATE_RETURN(buffer != nullptr, EINVAL, EOF);
    _VALIDATE_RETURN(format != nullptr, EINVAL, EOF);

    size_t const buffer_count_for_stream =
        char_traits::tcsnlen(buffer, buffer_count);

    _LocaleUpdate locale_update(locale);

    processor_type processor(
        string_input_adapter<Character>(buffer, buffer_count_for_stream),
        options,
        format,
        locale_update.GetLocaleT(),
        arglist);

    return processor.process();
  }

  extern "C" int __cdecl __stdio_common_vsscanf(unsigned __int64 const options,
                                                char const *const buffer,
                                                size_t const buffer_count,
                                                char const *const format,
                                                _locale_t const locale,
                                                va_list const arglist)
  {
    return common_vsscanf(
        options, buffer, buffer_count, format, locale, arglist);
  }

  extern "C" int __cdecl __stdio_common_vswscanf(unsigned __int64 const options,
                                                 wchar_t const *const buffer,
                                                 size_t const buffer_count,
                                                 wchar_t const *const format,
                                                 _locale_t const locale,
                                                 va_list const arglist)
  {
    return common_vsscanf(
        options, buffer, buffer_count, format, locale, arglist);
  }

} // namespace mingw_thunk::ucrt
