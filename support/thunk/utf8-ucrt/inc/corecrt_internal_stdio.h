#pragma once

#include "corecrt_internal.h"
#include "corecrt_internal_ptd_propagation.h"
#include "corecrt_internal_traits.h"

#include <errno.h>
#include <stdio.h>

#include <sal.h>

#include "../mingw/post_fix.h"

namespace mingw_thunk::ucrt
{

  _Check_return_opt_ wint_t __cdecl
  _fputwc_nolock_internal(_In_ wchar_t _Character,
                          _Inout_ FILE *_Stream,
                          _Inout_ __crt_cached_ptd_host &_Ptd);

  _Success_(return != EOF) _Check_return_opt_ int __cdecl
  _fputc_nolock_internal(_In_ int _Character,
                         _Inout_ FILE *_Stream,
                         _Inout_ __crt_cached_ptd_host &_Ptd);

  class __crt_stdio_stream
  {
  public:
    explicit __crt_stdio_stream(FILE *const stream) throw()
        : _stream(stream)
    {
    }

    bool valid() const noexcept
    {
      return _stream != nullptr;
    }

    FILE *public_stream() const noexcept
    {
      return _stream;
    }

  private:
    FILE *_stream;
  };

  template <typename Action>
  auto __acrt_lock_stream_and_call(FILE *const stream, Action &&action) noexcept
      -> decltype(action())
  {
    _lock_file(stream);

    struct guard
    {
      FILE *stream;

      ~guard() noexcept
      {
        _unlock_file(stream);
      }
    } guard{stream};

    return action();
  }

  class __acrt_stdio_temporary_buffering_guard
  {
  public:
    explicit __acrt_stdio_temporary_buffering_guard(FILE *const stream,
                                                    __crt_cached_ptd_host &ptd)
        : _stream(stream), _ptd(ptd)
    {
      _batch = console_utf8_begin(stream);
    }

    ~__acrt_stdio_temporary_buffering_guard() noexcept
    {
      console_utf8_end(_batch, _ptd);
    }

  private:
    FILE *_stream;
    __crt_cached_ptd_host &_ptd;
    bool _flag;

    console_utf8_batch _batch{};
  };

  template <typename Character>
  struct __acrt_stdio_char_traits;

  template <>
  struct __acrt_stdio_char_traits<char> : __crt_char_traits<char>
  {
    static int_type const eof = EOF;

    static bool validate_stream_is_ansi_if_required(FILE *const stream) noexcept
    {
      return true;
    }
  };

  template <>
  struct __acrt_stdio_char_traits<wchar_t> : __crt_char_traits<wchar_t>
  {
    static int_type const eof = WEOF;

    static bool validate_stream_is_ansi_if_required(FILE *const stream) noexcept
    {
      return true;
    }
  };

} // namespace mingw_thunk::ucrt
