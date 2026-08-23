#pragma once

#include "corecrt_internal_fltintrn.h"
#include "corecrt_internal_mbstring.h"
#include "corecrt_internal_ptd_propagation.h"
#include "corecrt_internal_stdio.h"
#include "corecrt_internal_strtox.h"

#include "../vcruntime/internal_shared.h"

#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>

namespace mingw_thunk::ucrt::__crt_stdio_output
{

  template <typename T>
  T read_va_arg(va_list &arglist) noexcept
  {
    return va_arg(arglist, T);
  }

  template <typename T>
  T peek_va_arg(va_list arglist) noexcept
  {
    return va_arg(arglist, T);
  }

  template <typename Character, typename Derived>
  class output_adapter_common
  {
  public:
    void write_character(Character const c,
                         int *const count_written,
                         __crt_cached_ptd_host &ptd) const noexcept
    {
      if (static_cast<Derived const *>(this)
              ->write_character_without_count_update(c, ptd))
      {
        ++*count_written;
      }
      else
      {
        *count_written = -1;
      }
    }

  protected:
    void write_string_impl(Character const *const string,
                           int const length,
                           int *const count_written,
                           __crt_cached_ptd_host &ptd) const noexcept
    {
      auto const reset_errno = ptd.get_errno().create_guard();

      Character const *const string_last{string + length};
      for (Character const *it{string}; it != string_last; ++it)
      {
        if (static_cast<Derived const *>(this)
                ->write_character_without_count_update(*it, ptd))
        {
          ++*count_written;
        }
        else
        {
          if (!ptd.get_errno().check(EILSEQ))
          {
            *count_written = -1;
            break;
          }

          write_character('?', count_written, ptd);
        }
      }
    }
  };

  template <typename Character>
  class console_output_adapter
      : public output_adapter_common<Character,
                                     console_output_adapter<Character>>
  {
  public:
    typedef __acrt_stdio_char_traits<Character> char_traits;

    bool validate(__crt_cached_ptd_host &) const throw()
    {
      return true;
    }

    bool write_character_without_count_update(Character const c,
                                              __crt_cached_ptd_host &ptd) const
        throw()
    {
      return char_traits::puttch_nolock_internal(c, ptd) != char_traits::eof;
    }

    void write_string(Character const *const string,
                      int const length,
                      int *const count_written,
                      __crt_cached_ptd_host &ptd) const throw()
    {
      using b =
          output_adapter_common<Character, console_output_adapter<Character>>;

      b::write_string_impl(string, length, count_written, ptd);
    }
  };

  template <typename Character>
  class stream_output_adapter
      : public output_adapter_common<Character,
                                     stream_output_adapter<Character>>
  {
  public:
    typedef __acrt_stdio_char_traits<Character> char_traits;

    stream_output_adapter(FILE *const public_stream) throw()
        : _stream{public_stream}
    {
    }

    bool validate(__crt_cached_ptd_host &ptd) const throw()
    {
      _UCRT_VALIDATE_RETURN(ptd, _stream.valid(), EINVAL, false);

      return char_traits::validate_stream_is_ansi_if_required(
          _stream.public_stream());
    }

    bool write_character_without_count_update(Character const c,
                                              __crt_cached_ptd_host &ptd) const
        throw()
    {
      // UCRT checks for string-backed stream leftover from legacy VCRT.

      return char_traits::puttc_nolock_internal(
                 c, _stream.public_stream(), ptd) != char_traits::eof;
    }

    void write_string(Character const *const string,
                      int const length,
                      int *const count_written,
                      __crt_cached_ptd_host &ptd) const throw()
    {
      using b =
          output_adapter_common<Character, stream_output_adapter<Character>>;

      // UCRT checks for string-backed stream leftover from legacy VCRT.

      b::write_string_impl(string, length, count_written, ptd);
    }

  private:
    __crt_stdio_stream _stream;
  };

  template <typename Character>
  struct string_output_adapter_context
  {
    Character *_buffer;
    size_t _buffer_count;
    size_t _buffer_used;
    bool _continue_count;
  };

  template <typename Character>
  class string_output_adapter
  {
  public:
    typedef __acrt_stdio_char_traits<Character> char_traits;
    typedef string_output_adapter_context<Character> context_type;

    string_output_adapter(context_type *const context) noexcept
        : _context(context)
    {
    }

    bool validate(__crt_cached_ptd_host &ptd) const noexcept
    {
      _UCRT_VALIDATE_RETURN(ptd, _context != nullptr, EINVAL, false);
      return true;
    }

    __forceinline bool write_character(Character const c,
                                       int *const count_written,
                                       __crt_cached_ptd_host &) const throw()
    {
      if (_context->_buffer_used == _context->_buffer_count)
      {
        if (_context->_continue_count)
        {
          ++*count_written;
        }
        else
        {
          *count_written = -1;
        }

        return _context->_continue_count;
      }

      ++*count_written;
      ++_context->_buffer_used;
      *_context->_buffer++ = c;
      return true;
    }

    void write_string(Character const *const string,
                      int const length,
                      int *const count_written,
                      __crt_cached_ptd_host &ptd) const noexcept
    {
      if (length == 0)
      {
        return;
      }

      if (_context->_buffer_used == _context->_buffer_count)
      {
        if (_context->_continue_count)
        {
          *count_written += length;
        }
        else
        {
          *count_written = -1;
        }

        return;
      }

      size_t const space_available =
          _context->_buffer_count - _context->_buffer_used;
      size_t const elements_to_copy =
          __min(space_available, static_cast<size_t>(length));

      memcpy(_context->_buffer, string, elements_to_copy * sizeof(Character));

      _context->_buffer += elements_to_copy;
      _context->_buffer_used += elements_to_copy;

      if (_context->_continue_count)
      {
        *count_written += length;
      }
      else if (elements_to_copy != static_cast<size_t>(length))
      {
        *count_written = -1;
      }
      else
      {
        *count_written += static_cast<int>(elements_to_copy);
      }
    }

  private:
    context_type *_context;
  };

  template <typename OutputAdapter, typename Character>
  __forceinline void
  write_multiple_characters(OutputAdapter const &adapter,
                            Character const c,
                            int const count,
                            int *const count_written,
                            __crt_cached_ptd_host &ptd) throw()
  {
    for (int i{0}; i < count; ++i)
    {
      adapter.write_character(c, count_written, ptd);
      if (*count_written == -1)
        break;
    }
  }

  class formatting_buffer
  {
  public:
    enum
    {
      member_buffer_size = 1024,
    };

    static_assert(member_buffer_size >= (_CVTBUFSIZE + 6) * 2,
                  "Buffer is too small");

    formatting_buffer() throw()
        : _dynamic_buffer_size{0}
    {
    }

    template <typename T>
    bool ensure_buffer_is_big_enough(size_t const count,
                                     __crt_cached_ptd_host &ptd) throw()
    {
      constexpr size_t max_count = SIZE_MAX / sizeof(T) / 2;
      _UCRT_VALIDATE_RETURN_NOEXC(ptd, max_count >= count, ENOMEM, false);

      size_t const required_size{count * sizeof(T) * 2};

      if (!_dynamic_buffer && required_size <= member_buffer_size)
      {
        return true;
      }

      if (required_size <= _dynamic_buffer_size)
      {
        return true;
      }

      __crt_unique_heap_ptr<char> new_buffer{(char *)malloc(required_size)};
      if (!new_buffer)
      {
        return false;
      }

      _dynamic_buffer = static_cast<__crt_unique_heap_ptr<char> &&>(new_buffer);
      _dynamic_buffer_size = required_size;
      return true;
    }

    template <typename T>
    T *data() throw()
    {
      if (!_dynamic_buffer)
        return reinterpret_cast<T *>(_member_buffer);

      return reinterpret_cast<T *>(_dynamic_buffer.get());
    }

    template <typename T>
    T *scratch_data() throw()
    {
      if (!_dynamic_buffer)
        return reinterpret_cast<T *>(_member_buffer) + count<T>();

      return reinterpret_cast<T *>(_dynamic_buffer.get()) + count<T>();
    }

    template <typename T>
    size_t count() const throw()
    {
      if (!_dynamic_buffer)
        return member_buffer_size / sizeof(T) / 2;

      return _dynamic_buffer_size / sizeof(T) / 2;
    }

    template <typename T>
    size_t scratch_count() const throw()
    {
      return count<T>();
    }

  private:
    char _member_buffer[member_buffer_size];

    size_t _dynamic_buffer_size;
    __crt_unique_heap_ptr<char> _dynamic_buffer;
  };

  inline void __cdecl force_decimal_point(_Inout_z_ char *buffer) throw()
  {
    if (_tolower_fast_internal(static_cast<unsigned char>(*buffer)) != 'e')
    {
      do
      {
        ++buffer;
      } while (_isdigit_fast_internal(static_cast<unsigned char>(*buffer)));
    }

    if (_tolower_fast_internal(*buffer) == 'x')
    {
      buffer += 2;
    }

    char holdchar = *buffer;

    *buffer++ = '.';

    do
    {
      char const nextchar = *buffer;
      *buffer = holdchar;
      holdchar = nextchar;
    } while (*buffer++);
  }

  inline void __cdecl crop_zeroes(_Inout_z_ char *buffer) throw()
  {
    char const decimal_point = '.';

    while (*buffer && *buffer != decimal_point)
      ++buffer;

    if (*buffer++)
    {
      while (*buffer && *buffer != 'e' && *buffer != 'E')
        ++buffer;

      char *stop = buffer--;

      while (*buffer == '0')
        --buffer;

      if (*buffer == decimal_point)
        --buffer;

      while ((*++buffer = *stop++) != '\0')
      {
      }
    }
  }

  enum class state : unsigned char
  {
    normal,
    percent,
    flag,
    width,
    dot,
    precision,
    size,
    type,
    invalid,
  };

  enum class character_type : unsigned char
  {
    other,
    percent,
    dot,
    star,
    zero,
    digit,
    flag,
    size,
    type,
  };

  struct state_transition_pair
  {
    state next_state;
    character_type current_class;
  };

  template <typename T, size_t Size>
  class spectre_mitigated_lookup_table
  {
  public:
    static size_t const mask = Size - 1;
    static_assert((Size & mask) == 0, "Size must be a power of two.");

    T const &operator[](size_t const index) const
    {
      return m_array[index & mask];
    }

    T m_array[Size];
  };

  using printf_state_transition_table =
      spectre_mitigated_lookup_table<state_transition_pair, 128>;

  inline printf_state_transition_table const standard_lookup_table_spectre{
      state::normal,    character_type::flag,
      state::normal,    character_type::other,
      state::normal,    character_type::other,
      state::normal,    character_type::flag,
      state::normal,    character_type::other,
      state::normal,    character_type::percent,
      state::normal,    character_type::other,
      state::normal,    character_type::other,
      state::percent,   character_type::other,
      state::normal,    character_type::other,
      state::normal,    character_type::star,
      state::normal,    character_type::flag,
      state::normal,    character_type::other,
      state::normal,    character_type::flag,
      state::normal,    character_type::dot,
      state::percent,   character_type::other,
      state::normal,    character_type::zero,
      state::dot,       character_type::digit,
      state::dot,       character_type::digit,
      state::dot,       character_type::digit,
      state::normal,    character_type::digit,
      state::normal,    character_type::digit,
      state::normal,    character_type::digit,
      state::normal,    character_type::digit,
      state::normal,    character_type::digit,
      state::width,     character_type::digit,
      state::width,     character_type::other,
      state::normal,    character_type::other,
      state::precision, character_type::other,
      state::normal,    character_type::other,
      state::normal,    character_type::other,
      state::normal,    character_type::other,
      state::normal,    character_type::other,
      state::flag,      character_type::type,
      state::flag,      character_type::other,
      state::width,     character_type::type,
      state::precision, character_type::other,
      state::precision, character_type::type,
      state::normal,    character_type::size,
      state::normal,    character_type::type,
      state::normal,    character_type::other,
      state::width,     character_type::size,
      state::width,     character_type::other,
      state::width,     character_type::other,
      state::precision, character_type::size,
      state::precision, character_type::other,
      state::normal,    character_type::size,
      state::normal,    character_type::other,
      state::normal,    character_type::other,
      state::flag,      character_type::other,
      state::flag,      character_type::other,
      state::normal,    character_type::type,
      state::normal,    character_type::size,
      state::normal,    character_type::other,
      state::normal,    character_type::other,
      state::normal,    character_type::other,
      state::normal,    character_type::type,
      state::size,      character_type::other,
      state::size,      character_type::type,
      state::size,      character_type::other,
      state::size,      character_type::other,
      state::size,      character_type::other,
      state::size,      character_type::other,
      state::normal,    character_type::other,
      state::normal,    character_type::other,
      state::type,      character_type::type,
      state::type,      character_type::other,
      state::type,      character_type::type,
      state::type,      character_type::type,
      state::type,      character_type::type,
      state::type,      character_type::type,
      state::normal,    character_type::type,
      state::normal,    character_type::size,
      state::normal,    character_type::type,
      state::normal,    character_type::size,
      state::normal,    character_type::other,
      state::normal,    character_type::size,
      state::normal,    character_type::other,
      state::normal,    character_type::type,
      state::normal,    character_type::type,
      state::normal,    character_type::type,
      state::normal,    character_type::other,
      state::normal,    character_type::other,
      state::normal,    character_type::type,
      state::normal,    character_type::size,
      state::normal,    character_type::type,
      state::normal,    character_type::other,
      state::normal,    character_type::size,
      state::normal,    character_type::type,
      state::normal,    character_type::other,
      state::normal,    character_type::size};

  inline printf_state_transition_table const
      format_validation_lookup_table_spectre{
          state::normal,    character_type::flag,
          state::invalid,   character_type::other,
          state::invalid,   character_type::other,
          state::invalid,   character_type::flag,
          state::invalid,   character_type::other,
          state::invalid,   character_type::percent,
          state::invalid,   character_type::other,
          state::normal,    character_type::other,
          state::normal,    character_type::other,
          state::percent,   character_type::other,
          state::normal,    character_type::star,
          state::invalid,   character_type::flag,
          state::invalid,   character_type::other,
          state::invalid,   character_type::flag,
          state::invalid,   character_type::dot,
          state::invalid,   character_type::other,
          state::percent,   character_type::zero,
          state::normal,    character_type::digit,
          state::normal,    character_type::digit,
          state::dot,       character_type::digit,
          state::dot,       character_type::digit,
          state::dot,       character_type::digit,
          state::invalid,   character_type::digit,
          state::invalid,   character_type::digit,
          state::invalid,   character_type::digit,
          state::normal,    character_type::digit,
          state::normal,    character_type::other,
          state::normal,    character_type::other,
          state::width,     character_type::other,
          state::width,     character_type::other,
          state::invalid,   character_type::other,
          state::precision, character_type::other,
          state::invalid,   character_type::other,
          state::invalid,   character_type::type,
          state::normal,    character_type::other,
          state::normal,    character_type::type,
          state::normal,    character_type::other,
          state::flag,      character_type::type,
          state::flag,      character_type::size,
          state::width,     character_type::type,
          state::precision, character_type::other,
          state::precision, character_type::size,
          state::invalid,   character_type::other,
          state::normal,    character_type::other,
          state::normal,    character_type::size,
          state::normal,    character_type::other,
          state::width,     character_type::size,
          state::width,     character_type::other,
          state::width,     character_type::other,
          state::precision, character_type::other,
          state::precision, character_type::other,
          state::invalid,   character_type::type,
          state::normal,    character_type::size,
          state::normal,    character_type::other,
          state::normal,    character_type::other,
          state::flag,      character_type::other,
          state::flag,      character_type::type,
          state::invalid,   character_type::other,
          state::invalid,   character_type::type,
          state::invalid,   character_type::other,
          state::invalid,   character_type::other,
          state::normal,    character_type::other,
          state::normal,    character_type::other,
          state::normal,    character_type::other,
          state::size,      character_type::other,
          state::size,      character_type::type,
          state::size,      character_type::other,
          state::size,      character_type::type,
          state::size,      character_type::type,
          state::size,      character_type::type,
          state::normal,    character_type::type,
          state::normal,    character_type::type,
          state::normal,    character_type::size,
          state::type,      character_type::type,
          state::type,      character_type::size,
          state::type,      character_type::other,
          state::type,      character_type::size,
          state::type,      character_type::other,
          state::type,      character_type::other,
          state::normal,    character_type::type,
          state::normal,    character_type::type,
          state::normal,    character_type::other,
          state::normal,    character_type::other,
          state::normal,    character_type::type,
          state::normal,    character_type::size,
          state::normal,    character_type::type,
          state::normal,    character_type::other,
          state::normal,    character_type::size,
          state::normal,    character_type::type,
          state::normal,    character_type::other,
          state::normal,    character_type::size};

  enum FLAG : unsigned
  {
    FL_SIGN = 0x01,
    FL_SIGNSP = 0x02,
    FL_LEFT = 0x04,
    FL_LEADZERO = 0x08,
    FL_SIGNED = 0x10,
    FL_ALTERNATE = 0x20,
    FL_NEGATIVE = 0x40,
    FL_FORCEOCTAL = 0x80,
  };

  enum class length_modifier
  {
    none,
    hh,
    h,
    l,
    ll,
    j,
    z,
    t,
    L,
    I,
    I32,
    I64,
    w,
    T,
    enumerator_count
  };

  inline size_t __cdecl to_integer_size(length_modifier const length) throw()
  {
    switch (length)
    {
    case length_modifier::none:
      return sizeof(int);
    case length_modifier::hh:
      return sizeof(char);
    case length_modifier::h:
      return sizeof(short);
    case length_modifier::l:
      return sizeof(long);
    case length_modifier::ll:
      return sizeof(long long);
    case length_modifier::j:
      return sizeof(intmax_t);
    case length_modifier::z:
      return sizeof(size_t);
    case length_modifier::t:
      return sizeof(ptrdiff_t);
    case length_modifier::I:
      return sizeof(void *);
    case length_modifier::I32:
      return sizeof(int32_t);
    case length_modifier::I64:
      return sizeof(int64_t);
    default:
      return 0;
    }
  }

  template <typename Character>
  bool __cdecl is_wide_character_specifier(uint64_t const options,
                                           Character const format_type,
                                           length_modifier const length) throw()
  {
    switch (length)
    {
    case length_modifier::l:
      return true;
    case length_modifier::w:
      return true;
    case length_modifier::h:
      return false;
    }

    if (length == length_modifier::T)
    {
      return sizeof(Character) == sizeof(wchar_t);
    }

    bool const is_naturally_wide{
        sizeof(Character) == sizeof(wchar_t) &&
        (options & _CRT_INTERNAL_PRINTF_LEGACY_WIDE_SPECIFIERS) != 0};

    bool const is_natural_width{format_type == 'c' || format_type == 's'};

    return is_naturally_wide == is_natural_width;
  }

  template <typename Character>
  class common_data
  {
  protected:
    common_data(__crt_cached_ptd_host &ptd) noexcept
        : _options{0}
        , _ptd{ptd}
        , _format_it{nullptr}
        , _valist_it{nullptr}
        , _characters_written{0}
        , _state{state::normal}
        , _flags{0}
        , _field_width{0}
        , _precision{0}
        , _suppress_output{false}
        , _format_char{'\0'}
        , _string_length{0}
        , _string_is_wide{false}
    {
    }

    uint64_t _options;

    __crt_cached_ptd_host &_ptd;

    Character const *_format_it;
    va_list _valist_it;

    int _characters_written;

    state _state;
    unsigned _flags;
    int _field_width;
    int _precision;
    length_modifier _length;
    bool _suppress_output;

    Character _format_char;

    union
    {
      char *_narrow_string;
      wchar_t *_wide_string;
    };

    char *&tchar_string(char) noexcept
    {
      return _narrow_string;
    }

    wchar_t *&tchar_string(wchar_t) noexcept
    {
      return _wide_string;
    }

    Character *&tchar_string() noexcept
    {
      return tchar_string(Character());
    }

    int _string_length;
    bool _string_is_wide;

    formatting_buffer _buffer;
  };

  template <typename Character, typename OutputAdapter>
  class output_adapter_data : protected common_data<Character>
  {
  protected:
    output_adapter_data(OutputAdapter const &output_adapter,
                        uint64_t const options,
                        Character const *const format,
                        __crt_cached_ptd_host &ptd,
                        va_list const arglist) throw()
        : common_data<Character>{ptd}
        , _output_adapter(output_adapter)
    {
      using base = common_data<Character>;

      base::_options = options;
      base::_format_it = format;
      base::_valist_it = arglist;
    }

    OutputAdapter _output_adapter;
  };

  template <typename Character, typename OutputAdapter>
  class standard_base : protected output_adapter_data<Character, OutputAdapter>
  {
  protected:
    template <typename... Ts>
    standard_base(Ts &&...arguments) noexcept
        : output_adapter_data<Character, OutputAdapter>{arguments...}
        , _current_pass{pass::not_started}
    {
    }

    bool advance_to_next_pass() noexcept
    {
      _current_pass =
          static_cast<pass>(static_cast<unsigned>(_current_pass) + 1);
      return _current_pass != pass::finished;
    }

    bool validate_and_update_state_at_end_of_format_string() const noexcept
    {
      return true;
    }

    bool should_format() noexcept
    {
      return true;
    }

    template <typename RequestedParameterType, typename ActualParameterType>
    bool extract_argument_from_va_list(ActualParameterType &result) noexcept
    {
      using base = output_adapter_data<Character, OutputAdapter>;

      result = static_cast<ActualParameterType>(
          read_va_arg<RequestedParameterType>(base::_valist_it));

      return true;
    }

    bool update_field_width() noexcept
    {
      using base = output_adapter_data<Character, OutputAdapter>;

      base::_field_width = read_va_arg<int>(base::_valist_it);
      return true;
    }

    bool update_precision() noexcept
    {
      using base = output_adapter_data<Character, OutputAdapter>;

      base::_precision = read_va_arg<int>(base::_valist_it);
      return true;
    }

    bool validate_state_for_type_case_a() const noexcept
    {
      return true;
    }

    bool should_skip_normal_state_processing() noexcept
    {
      return false;
    }

    bool validate_and_update_state_at_beginning_of_format_character() noexcept
    {
      return true;
    }

    bool should_skip_type_state_output() const noexcept
    {
      return false;
    }

    static unsigned state_count() noexcept
    {
      return static_cast<unsigned>(state::type) + 1;
    }

    static printf_state_transition_table const &
    state_transition_table() noexcept
    {
      return standard_lookup_table_spectre;
    }

  private:
    enum class pass : unsigned
    {
      not_started,
      output,
      finished
    };

    pass _current_pass;
  };

  template <typename Character, typename OutputAdapter>
  class format_validation_base
      : protected standard_base<Character, OutputAdapter>
  {
  protected:
    template <typename... Ts>
    format_validation_base(Ts &&...arguments) noexcept
        : standard_base<Character, OutputAdapter>{arguments...}
    {
    }

    bool validate_and_update_state_at_end_of_format_string() noexcept
    {
      using b = standard_base<Character, OutputAdapter>;

      _UCRT_VALIDATE_RETURN(b::_ptd,
                            b::_state == state::normal ||
                                b::_state == state::type,
                            EINVAL,
                            false);

      return true;
    }

    static unsigned state_count() noexcept
    {
      return static_cast<unsigned>(state::invalid) + 1;
    }

    static printf_state_transition_table const &
    state_transition_table() noexcept
    {
      return format_validation_lookup_table_spectre;
    }
  };

  template <typename Character, typename OutputAdapter>
  class positional_parameter_base
      : protected format_validation_base<Character, OutputAdapter>
  {
  protected:
    typedef positional_parameter_base self_type;
    typedef format_validation_base<Character, OutputAdapter> base_type;
    typedef __crt_char_traits<Character> char_traits;

    template <typename... Ts>
    positional_parameter_base(Ts &&...arguments) throw()
        : format_validation_base<Character, OutputAdapter>{arguments...}
        , _current_pass{pass::not_started}
        , _format_mode{mode::unknown}
        , _format{base_type::_format_it}
        , _type_index{-1}
        , _maximum_index{-1}
    {
    }

  private:
    enum class pass : unsigned
    {
      not_started,
      position_scan,
      output,
      finished
    };

    enum class mode : unsigned
    {
      unknown,
      nonpositional,
      positional
    };

    enum class parameter_type : unsigned
    {
      unused,
      int32,
      int64,
      pointer,
      real64
    };

    struct parameter_data
    {
      parameter_type _actual_type;
      Character _format_type;

      va_list _valist_it;
      length_modifier _length;
    };

    template <typename T>
    static parameter_type __cdecl get_parameter_type(T *) throw()
    {
      return parameter_type::pointer;
    }
    static parameter_type __cdecl get_parameter_type(short) throw()
    {
      return parameter_type::int32;
    }
    static parameter_type __cdecl get_parameter_type(unsigned short) throw()
    {
      return parameter_type::int32;
    }
    static parameter_type __cdecl get_parameter_type(wchar_t) throw()
    {
      return parameter_type::int32;
    }
    static parameter_type __cdecl get_parameter_type(int) throw()
    {
      return parameter_type::int32;
    }
    static parameter_type __cdecl get_parameter_type(unsigned int) throw()
    {
      return parameter_type::int32;
    }
    static parameter_type __cdecl get_parameter_type(__int64) throw()
    {
      return parameter_type::int64;
    }
    static parameter_type __cdecl get_parameter_type(unsigned __int64) throw()
    {
      return parameter_type::int64;
    }
    static parameter_type __cdecl get_parameter_type(_CRT_DOUBLE) throw()
    {
      return parameter_type::real64;
    }

    pass _current_pass;
    mode _format_mode;

    Character const *_format;

    parameter_data _parameters[_ARGMAX];

    int _maximum_index;
    int _type_index;
  };

  template <typename Character, typename OutputAdapter, typename ProcessorBase>
  class output_processor : private ProcessorBase
  {
  public:
    typedef __acrt_stdio_char_traits<Character> char_traits;

    output_processor(OutputAdapter const &output_adapter,
                     uint64_t const options,
                     Character const *const format,
                     __crt_cached_ptd_host &ptd,
                     va_list const arglist) noexcept
        : ProcessorBase{output_adapter, options, format, ptd, arglist}
    {
    }

    int process() noexcept
    {
      using b = ProcessorBase;

      if (!b::_output_adapter.validate(b::_ptd))
      {
        return -1;
      }

      _UCRT_VALIDATE_RETURN(b::_ptd, b::_format_it != nullptr, EINVAL, -1);

      while (b::advance_to_next_pass())
      {
        b::_string_length = 0;
        b::_state = state::normal;

        while ((b::_format_char = *b::_format_it++) != '\0' &&
               b::_characters_written >= 0)
        {
          b::_state = find_next_state(b::_format_char, b::_state);

          if (!b::validate_and_update_state_at_beginning_of_format_character())
          {
            return -1;
          }

          if (b::_state >= state::invalid)
          {
            _UCRT_VALIDATE_RETURN(
                b::_ptd, ("Incorrect format specifier", 0), EINVAL, -1);
          }

          bool result = false;

          switch (b::_state)
          {
          case state::normal:
            result = state_case_normal();
            break;
          case state::percent:
            result = state_case_percent();
            break;
          case state::flag:
            result = state_case_flag();
            break;
          case state::width:
            result = state_case_width();
            break;
          case state::dot:
            result = state_case_dot();
            break;
          case state::precision:
            result = state_case_precision();
            break;
          case state::size:
            result = state_case_size();
            break;
          case state::type:
            result = state_case_type();
            break;
          }

          if (!result)
            return -1;
        }

        if (!b::validate_and_update_state_at_end_of_format_string())
          return -1;
      }

      return b::_characters_written;
    }

  private:
    bool state_case_normal() noexcept
    {
      using b = ProcessorBase;

      if (b::should_skip_normal_state_processing())
        return true;

      _UCRT_VALIDATE_RETURN(b::_ptd, state_case_normal_common(), EINVAL, false);

      return true;
    }

    bool state_case_normal_common() noexcept
    {
      using b = ProcessorBase;

      if (!state_case_normal_tchar(Character()))
        return false;

      b::_output_adapter.write_character(
          b::_format_char, &(b::_characters_written), b::_ptd);
      return true;
    }

    bool state_case_normal_tchar(char) noexcept
    {
      using b = ProcessorBase;

      b::_string_is_wide = false;

      // UCRT checks for DBCS leading byte. UTF-8 is always safe.

      return true;
    }

    bool state_case_normal_tchar(wchar_t) noexcept
    {
      ProcessorBase::_string_is_wide = true;
      return true;
    }

    bool state_case_percent() noexcept
    {
      using b = ProcessorBase;

      b::_field_width = 0;
      b::_suppress_output = false;
      b::_flags = 0;
      b::_precision = -1;
      b::_length = length_modifier::none;
      b::_string_is_wide = false;

      return true;
    }

    bool state_case_flag() noexcept
    {
      switch (ProcessorBase::_format_char)
      {
      case '-':
        set_flag(FL_LEFT);
        break;
      case '+':
        set_flag(FL_SIGN);
        break;
      case ' ':
        set_flag(FL_SIGNSP);
        break;
      case '#':
        set_flag(FL_ALTERNATE);
        break;
      case '0':
        set_flag(FL_LEADZERO);
        break;
      }

      return true;
    }

    bool parse_int_from_format_string(int *const result) noexcept
    {
      using b = ProcessorBase;

      auto const reset_errno = b::_ptd.get_errno().create_guard();

      Character *end{};
      *result = static_cast<int>(
          _tcstol_internal(b::_ptd, b::_format_it - 1, &end, 10));

      if (errno == ERANGE)
      {
        return false;
      }

      if (end < b::_format_it)
      {
        return false;
      }

      b::_format_it = end;
      return true;
    }

    bool state_case_width() noexcept
    {
      using b = ProcessorBase;

      if (b::_format_char != '*')
      {
        return parse_int_from_format_string(&(b::_field_width));
      }

      if (!b::update_field_width())
        return false;

      if (!b::should_format())
        return true;

      if (b::_field_width < 0)
      {
        set_flag(FL_LEFT);
        b::_field_width = -b::_field_width;
      }

      return true;
    }

    bool state_case_dot() noexcept
    {
      ProcessorBase::_precision = 0;

      return true;
    }

    bool state_case_precision() noexcept
    {
      using b = ProcessorBase;

      if (b::_format_char != '*')
      {
        return parse_int_from_format_string(&(b::_precision));
      }

      if (!b::update_precision())
        return false;

      if (!b::should_format())
        return true;

      if (b::_precision < 0)
        b::_precision = -1;

      return true;
    }

    bool state_case_size() noexcept
    {
      using b = ProcessorBase;

      if (b::_format_char == 'F')
      {
        if ((b::_options & _CRT_INTERNAL_PRINTF_LEGACY_MSVCRT_COMPATIBILITY) ==
            0)
        {
          b::_state = state::type;
          return state_case_type();
        }

        return true;
      }

      if (b::_format_char == 'N')
      {
        if ((b::_options & _CRT_INTERNAL_PRINTF_LEGACY_MSVCRT_COMPATIBILITY) ==
            0)
        {
          b::_state = state::invalid;
          _UCRT_VALIDATE_RETURN(b::_ptd,
                                ("N length modifier not specifier", false),
                                EINVAL,
                                false);
          return false;
        }

        return true;
      }

      _UCRT_VALIDATE_RETURN(
          b::_ptd, b::_length == length_modifier::none, EINVAL, false);

      switch (b::_format_char)
      {
      case 'h':
      {
        if (*b::_format_it == 'h')
        {
          ++b::_format_it;
          b::_length = length_modifier::hh;
        }
        else
        {
          b::_length = length_modifier::h;
        }

        return true;
      }

      case 'I':
      {
        if (*b::_format_it == '3' && *(b::_format_it + 1) == '2')
        {
          b::_format_it += 2;
          b::_length = length_modifier::I32;
        }
        else if (*b::_format_it == '6' && *(b::_format_it + 1) == '4')
        {
          b::_format_it += 2;
          b::_length = length_modifier::I64;
        }
        else if (*b::_format_it == 'd' || *b::_format_it == 'i' ||
                 *b::_format_it == 'o' || *b::_format_it == 'u' ||
                 *b::_format_it == 'x' || *b::_format_it == 'X')
        {
          b::_length = length_modifier::I;
        }

        return true;
      }

      case 'l':
      {
        if (*b::_format_it == 'l')
        {
          ++b::_format_it;
          b::_length = length_modifier::ll;
        }
        else
        {
          b::_length = length_modifier::l;
        }

        return true;
      }

      case 'L':
      {
        b::_length = length_modifier::L;
        return true;
      }

      case 'j':
      {
        b::_length = length_modifier::j;
        return true;
      }

      case 't':
      {
        b::_length = length_modifier::t;
        return true;
      }

      case 'z':
      {
        b::_length = length_modifier::z;
        return true;
      }

      case 'w':
      {
        b::_length = length_modifier::w;
        return true;
      }

      case 'T':
      {
        b::_length = length_modifier::T;
        return true;
      }
      }

      return true;
    }

    bool state_case_type() noexcept
    {
      using b = ProcessorBase;

      bool result{false};
      switch (b::_format_char)
      {
      case 'C':
      case 'c':
        result = type_case_c();
        break;

      case 'Z':
        result = type_case_Z();
        break;
      case 'S':
      case 's':
        result = type_case_s();
        break;

      case 'A':
      case 'E':
      case 'F':
      case 'G':
      case 'a':
      case 'e':
      case 'f':
      case 'g':
        result = type_case_a();
        break;

      case 'd':
      case 'i':
        result = type_case_d();
        break;
      case 'u':
        result = type_case_u();
        break;
      case 'o':
        result = type_case_o();
        break;
      case 'X':
        result = type_case_X();
        break;
      case 'x':
        result = type_case_x();
        break;
      case 'p':
        result = type_case_p();
        break;

      case 'n':
        result = type_case_n();
        break;
      }

      if (!result)
        return false;

      if (b::should_skip_type_state_output())
        return true;

      if (b::_suppress_output)
        return true;

      Character prefix[3]{};
      size_t prefix_length{0};

      if (has_flag(FL_SIGNED))
      {
        if (has_flag(FL_NEGATIVE))
        {
          prefix[prefix_length++] = '-';
        }
        else if (has_flag(FL_SIGN))
        {
          prefix[prefix_length++] = '+';
        }
        else if (has_flag(FL_SIGNSP))
        {
          prefix[prefix_length++] = ' ';
        }
      }

      bool const print_integer_0x{
          (b::_format_char == 'x' || b::_format_char == 'X') &&
          has_flag(FL_ALTERNATE)};
      bool const print_floating_point_0x{b::_format_char == 'a' ||
                                         b::_format_char == 'A'};

      if (print_integer_0x || print_floating_point_0x)
      {
        prefix[prefix_length++] = '0';
        prefix[prefix_length++] =
            adjust_hexit('x' - 'a' + '9' + 1,
                         b::_format_char == 'X' || b::_format_char == 'A');
      }

      int const padding =
          static_cast<int>(b::_field_width - b::_string_length - prefix_length);

      if (!has_flag(FL_LEFT | FL_LEADZERO))
      {
        write_multiple_characters(b::_output_adapter,
                                  ' ',
                                  padding,
                                  &(b::_characters_written),
                                  b::_ptd);
      }

      b::_output_adapter.write_string(prefix,
                                      static_cast<int>(prefix_length),
                                      &(b::_characters_written),
                                      b::_ptd);

      if (has_flag(FL_LEADZERO) && !has_flag(FL_LEFT))
      {
        write_multiple_characters(b::_output_adapter,
                                  '0',
                                  padding,
                                  &(b::_characters_written),
                                  b::_ptd);
      }

      write_stored_string_tchar(Character());

      if (b::_characters_written >= 0 && has_flag(FL_LEFT))
      {
        write_multiple_characters(b::_output_adapter,
                                  ' ',
                                  padding,
                                  &(b::_characters_written),
                                  b::_ptd);
      }

      return true;
    }

    bool type_case_c() noexcept
    {
      return type_case_c_tchar(Character());
    }

    bool type_case_c_tchar(char) noexcept
    {
      using b = ProcessorBase;

      if (is_wide_character_specifier(b::_options, b::_format_char, b::_length))
      {
        wchar_t wide_character{};
        if (!b::template extract_argument_from_va_list<wchar_t>(wide_character))
        {
          return false;
        }

        if (!b::should_format())
        {
          return true;
        }

        errno_t const status{_wctomb_internal(&(b::_string_length),
                                              b::_buffer.template data<char>(),
                                              b::_buffer.template count<char>(),
                                              wide_character,
                                              b::_ptd)};
        if (status != 0)
        {
          b::_suppress_output = true;
        }
      }
      else
      {
        if (!b::template extract_argument_from_va_list<unsigned short>(
                b::_buffer.template data<char>()[0]))
        {
          return false;
        }

        if (!b::should_format())
        {
          return true;
        }

        b::_string_length = 1;
      }

      b::_narrow_string = b::_buffer.template data<char>();
      return true;
    }

    bool type_case_c_tchar(wchar_t) throw()
    {
      using b = ProcessorBase;

      b::_string_is_wide = true;

      wchar_t wide_character{};
      if (!b::template extract_argument_from_va_list<wchar_t>(wide_character))
        return false;

      if (!b::should_format())
        return true;

      if (!is_wide_character_specifier(
              b::_options, b::_format_char, b::_length))
      {
        char const local_buffer[2]{static_cast<char>(wide_character & 0x00ff),
                                   '\0'};
        int const mbc_length{
            _mbtowc_internal(b::_buffer.template data<wchar_t>(),
                             local_buffer,
                             4 /* MB_CUR_MAX */,
                             b::_ptd)};
        if (mbc_length < 0)
        {
          b::_suppress_output = true;
        }
      }
      else
      {
        b::_buffer.template data<wchar_t>()[0] = wide_character;
      }

      b::_wide_string = b::_buffer.template data<wchar_t>();
      b::_string_length = 1;
      return true;
    }

    bool type_case_Z() noexcept
    {
      using b = ProcessorBase;

      struct ansi_string
      {
        unsigned short _length;
        unsigned short _maximum_length;
        char *_buffer;
      };

      ansi_string *string{};
      if (!b::template extract_argument_from_va_list<ansi_string *>(string))
        return false;

      if (!b::should_format())
        return true;

      if (!string || string->_buffer == nullptr)
      {
        b::_narrow_string = narrow_null_string();
        b::_string_length = static_cast<int>(strlen(b::_narrow_string));
        b::_string_is_wide = false;
      }
      else if (is_wide_character_specifier(
                   b::_options, b::_format_char, b::_length))
      {
        b::_wide_string = reinterpret_cast<wchar_t *>(string->_buffer);
        b::_string_length = string->_length / static_cast<int>(sizeof(wchar_t));
        b::_string_is_wide = true;
      }
      else
      {
        b::_narrow_string = string->_buffer;
        b::_string_length = string->_length;
        b::_string_is_wide = false;
      }

      return true;
    }

    bool type_case_s() noexcept
    {
      using b = ProcessorBase;

      if (!b::template extract_argument_from_va_list<char *>(b::_narrow_string))
        return false;

      if (!b::should_format())
        return true;

      int const maximum_length{(b::_precision == -1) ? INT_MAX : b::_precision};

      if (is_wide_character_specifier(b::_options, b::_format_char, b::_length))
      {
        if (!b::_wide_string)
          b::_wide_string = wide_null_string();

        b::_string_is_wide = true;
        b::_string_length =
            static_cast<int>(wcsnlen(b::_wide_string, maximum_length));
      }
      else
      {
        if (!b::_narrow_string)
          b::_narrow_string = narrow_null_string();

        b::_string_length = type_case_s_compute_narrow_string_length(
            maximum_length, Character());
      }

      return true;
    }

    int type_case_s_compute_narrow_string_length(int const maximum_length,
                                                 char) noexcept
    {
      return static_cast<int>(
          strnlen(ProcessorBase::_narrow_string, maximum_length));
    }

    int type_case_s_compute_narrow_string_length(int const maximum_length,
                                                 wchar_t) noexcept
    {
      // original UCRT implementation checks DBCS leading byte and advances 2.
      // that's incorrect (even in UCRT locale semantics), MWE:

      //   #include <stdio.h>
      //   #include <locale.h>
      //   int main(int argc, char *argv[])
      //   {
      //     setlocale(LC_ALL, ".utf8");
      //     int nprinted = wprintf(L"%hs，%hs\n", u8"你好", u8"世界");
      //     printf("%d\n", nprinted);
      //   }

      // output:

      //   你好-1

      int n{0};
      for (char const *p{ProcessorBase::_narrow_string};
           n < maximum_length && *p;
           ++p)
      {
        // here we simply count UTF-8 non-trailing bytes
        // invalid UTF-8 sequences will fail in write loop anyway
        if ((static_cast<unsigned char>(*p) & 0b1100'0000) != 0b1000'0000)
          ++n;
      }

      return n;
    }

    bool type_case_a() noexcept
    {
      using b = ProcessorBase;

      set_flag(FL_SIGNED);

      if (!b::validate_state_for_type_case_a())
        return false;

      if (!b::should_format())
        return true;

      if (b::_precision < 0)
      {
        if (b::_format_char == 'a' || b::_format_char == 'A')
        {
          b::_precision = 13;
        }
        else
        {
          b::_precision = 6;
        }
      }
      else if (b::_precision == 0 &&
               (b::_format_char == 'g' || b::_format_char == 'G'))
      {
        b::_precision = 1;
      }

      if (!b::_buffer.template ensure_buffer_is_big_enough<char>(
              _CVTBUFSIZE + b::_precision, b::_ptd))
      {
        b::_precision =
            static_cast<int>(b::_buffer.template count<char>() - _CVTBUFSIZE);
      }

      b::_narrow_string = b::_buffer.template data<char>();

      _CRT_DOUBLE tmp{};
      if (!b::template extract_argument_from_va_list<_CRT_DOUBLE>(tmp))
      {
        return false;
      }

      __acrt_fp_format(&tmp.x,
                       b::_buffer.template data<char>(),
                       b::_buffer.template count<char>(),
                       b::_buffer.template scratch_data<char>(),
                       b::_buffer.template scratch_count<char>(),
                       static_cast<char>(b::_format_char),
                       b::_precision,
                       b::_options,
                       __acrt_rounding_mode::standard,
                       b::_ptd);

      if (has_flag(FL_ALTERNATE) && b::_precision == 0)
      {
        force_decimal_point(b::_narrow_string);
      }

      if ((b::_format_char == 'g' || b::_format_char == 'G') &&
          !has_flag(FL_ALTERNATE))
      {
        crop_zeroes(b::_narrow_string);
      }

      if (*b::_narrow_string == '-')
      {
        set_flag(FL_NEGATIVE);
        ++b::_narrow_string;
      }

      if (*b::_narrow_string == 'i' || *b::_narrow_string == 'I' ||
          *b::_narrow_string == 'n' || *b::_narrow_string == 'N')
      {
        unset_flag(FL_LEADZERO);
        b::_format_char = 's';
      }

      b::_string_length = static_cast<int>(strlen(b::_narrow_string));

      return true;
    }

    bool type_case_d() noexcept
    {
      set_flag(FL_SIGNED);

      return type_case_integer<10>();
    }

    bool type_case_u() noexcept
    {
      return type_case_integer<10>();
    }

    bool type_case_o() noexcept
    {
      // If the alternate flag is set, we force a leading 0:
      if (has_flag(FL_ALTERNATE))
        set_flag(FL_FORCEOCTAL);

      return type_case_integer<8>();
    }

    bool type_case_X() noexcept
    {
      return type_case_integer<16>(true);
    }

    bool type_case_x() noexcept
    {
      return type_case_integer<16>();
    }

    bool type_case_p() noexcept
    {
      using b = ProcessorBase;

      b::_precision = 2 * sizeof(void *);

      b::_length =
          sizeof(void *) == 4 ? length_modifier::I32 : length_modifier::I64;

      return type_case_integer<16>(true);
    }

    template <unsigned Radix>
    bool type_case_integer(bool const capital_hexits = false) noexcept
    {
      using b = ProcessorBase;

      size_t const integer_size = to_integer_size(b::_length);

      __int64 original_number{};
      bool extraction_result{};
      switch (integer_size)
      {
      case sizeof(int8_t):
        extraction_result =
            has_flag(FL_SIGNED)
                ? b::template extract_argument_from_va_list<int8_t>(
                      original_number)
                : b::template extract_argument_from_va_list<uint8_t>(
                      original_number);
        break;
      case sizeof(int16_t):
        extraction_result =
            has_flag(FL_SIGNED)
                ? b::template extract_argument_from_va_list<int16_t>(
                      original_number)
                : b::template extract_argument_from_va_list<uint16_t>(
                      original_number);
        break;
      case sizeof(int32_t):
        extraction_result =
            has_flag(FL_SIGNED)
                ? b::template extract_argument_from_va_list<int32_t>(
                      original_number)
                : b::template extract_argument_from_va_list<uint32_t>(
                      original_number);
        break;
      case sizeof(int64_t):
        extraction_result =
            has_flag(FL_SIGNED)
                ? b::template extract_argument_from_va_list<int64_t>(
                      original_number)
                : b::template extract_argument_from_va_list<uint64_t>(
                      original_number);
        break;
      default:
        _UCRT_VALIDATE_RETURN(
            b::_ptd, ("Invalid integer length modifier", 0), EINVAL, false);
        break;
      }

      if (!extraction_result)
        return false;

      if (!b::should_format())
        return true;

      unsigned __int64 number{};

      if (has_flag(FL_SIGNED) && original_number < 0)
      {
        number = static_cast<unsigned __int64>(-original_number);
        set_flag(FL_NEGATIVE);
      }
      else
      {
        number = static_cast<unsigned __int64>(original_number);
      }

      if (b::_precision < 0)
      {
        b::_precision = 1;
      }
      else
      {
        unset_flag(FL_LEADZERO);
        b::_buffer.template ensure_buffer_is_big_enough<Character>(
            b::_precision, b::_ptd);
      }

      if (number == 0)
      {
        unset_flag(FL_ALTERNATE);
      }

      b::_string_is_wide = sizeof(Character) == sizeof(wchar_t);

      if (integer_size == sizeof(int64_t))
      {
        type_case_integer_parse_into_buffer<uint64_t, Radix>(number,
                                                             capital_hexits);
      }
      else
      {
        type_case_integer_parse_into_buffer<uint32_t, Radix>(
            static_cast<uint32_t>(number), capital_hexits);
      }

      if (has_flag(FL_FORCEOCTAL) &&
          (b::_string_length == 0 || b::tchar_string()[0] != '0'))
      {
        *--b::tchar_string() = '0';
        ++b::_string_length;
      }

      return true;
    }

    template <typename UnsignedInteger, unsigned Radix>
    void type_case_integer_parse_into_buffer(UnsignedInteger number,
                                             bool const capital_hexits) noexcept
    {
      using b = ProcessorBase;

      Character *const last_digit{b::_buffer.template data<Character>() +
                                  b::_buffer.template count<Character>() - 1};

      Character *&string_pointer = b::tchar_string();

      string_pointer = last_digit;
      while (b::_precision > 0 || number != 0)
      {
        --b::_precision;

        Character digit{static_cast<Character>(number % Radix + '0')};
        number /= Radix;

        if (digit > '9')
        {
          digit = adjust_hexit(digit, capital_hexits);
        }

        *string_pointer-- = static_cast<char>(digit);
      }

      b::_string_length = static_cast<int>(last_digit - string_pointer);
      ++string_pointer;
    }

    bool type_case_n() noexcept
    {
      using b = ProcessorBase;

      void *p{nullptr};
      if (!b::template extract_argument_from_va_list<void *>(p))
        return false;

      if (!b::should_format())
        return true;

      if (!_get_printf_count_output())
      {
        _UCRT_VALIDATE_RETURN(
            b::_ptd, ("'n' format specifier disabled", 0), EINVAL, false);
        return false;
      }

      switch (to_integer_size(b::_length))
      {
      case sizeof(int8_t):
        *static_cast<int8_t *>(p) = static_cast<int8_t>(b::_characters_written);
        break;
      case sizeof(int16_t):
        *static_cast<int16_t *>(p) =
            static_cast<int16_t>(b::_characters_written);
        break;
      case sizeof(int32_t):
        *static_cast<int32_t *>(p) =
            static_cast<int32_t>(b::_characters_written);
        break;
      case sizeof(int64_t):
        *static_cast<int64_t *>(p) =
            static_cast<int64_t>(b::_characters_written);
        break;
      default:
        _UCRT_VALIDATE_RETURN(
            b::_ptd, ("Invalid integer length modifier", 0), EINVAL, false);
        break;
      }

      b::_suppress_output = true;
      return true;
    }

    bool write_stored_string_tchar(char) throw()
    {
      using b = ProcessorBase;

      if (!b::_string_is_wide || b::_string_length <= 0)
      {
        b::_output_adapter.write_string(b::_narrow_string,
                                        b::_string_length,
                                        &(b::_characters_written),
                                        b::_ptd);
      }
      else
      {
        wchar_t *p{b::_wide_string};
        for (int i{0}; i != b::_string_length; ++i)
        {
          char local_buffer[MB_LEN_MAX + 1];

          int mbc_length{0};
          errno_t const status{_wctomb_internal(&mbc_length,
                                                local_buffer,
                                                _countof(local_buffer),
                                                *p++,
                                                b::_ptd)};
          if (status != 0 || mbc_length == 0)
          {
            b::_characters_written = -1;
            return true;
          }

          b::_output_adapter.write_string(
              local_buffer, mbc_length, &(b::_characters_written), b::_ptd);
        }
      }

      return true;
    }

    bool write_stored_string_tchar(wchar_t) throw()
    {
      using b = ProcessorBase;

      if (b::_string_is_wide || b::_string_length <= 0)
      {
        b::_output_adapter.write_string(b::_wide_string,
                                        b::_string_length,
                                        &(b::_characters_written),
                                        b::_ptd);
      }
      else
      {
        char *p{b::_narrow_string};
        for (int i{0}; i != b::_string_length; ++i)
        {
          wchar_t wide_character{};
          int mbc_length{_mbtowc_internal(
              &wide_character, p, 4 /* MB_CUR_MAX */, b::_ptd)};

          if (mbc_length <= 0)
          {
            b::_characters_written = -1;
            return true;
          }

          b::_output_adapter.write_character(
              wide_character, &(b::_characters_written), b::_ptd);
          p += mbc_length;
        }
      }

      return true;
    }

    bool has_flag(unsigned const f) const noexcept
    {
      return (ProcessorBase::_flags & f) != 0;
    }

    void set_flag(unsigned const f) noexcept
    {
      ProcessorBase::_flags |= f;
    }

    void unset_flag(unsigned const f) noexcept
    {
      ProcessorBase::_flags &= ~f;
    }

    state find_next_state(Character const c, state const previous_state) const
        throw()
    {
      using b = ProcessorBase;

      auto const &lookup_table = b::state_transition_table();

      unsigned const current_class = static_cast<unsigned>(
          (c < ' ' || c > 'z') ? character_type::other
                               : static_cast<character_type>(
                                     lookup_table[c - ' '].current_class));

      auto const index = current_class * b::state_count() +
                         static_cast<unsigned>(previous_state);
      return static_cast<state>(lookup_table[index].next_state);
    }

    static char __cdecl adjust_hexit(int const value,
                                     bool const capitalize) noexcept
    {
      int const base{capitalize ? 'A' : 'a'};
      int const offset{base - '9' - 1};

      return static_cast<char>(offset + value);
    }

    static char *__cdecl narrow_null_string() noexcept
    {
      return "(null)";
    }

    static wchar_t *__cdecl wide_null_string() noexcept
    {
      return L"(null)";
    }
  };

} // namespace mingw_thunk::ucrt::__crt_stdio_output
