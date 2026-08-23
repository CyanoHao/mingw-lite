#include "../inc/corecrt_internal_big_integer.h"
#include "../inc/corecrt_internal_fltintrn.h"

#include <fenv.h>
#include <math.h>
#include <string.h>

#include <crtdbg.h>

namespace mingw_thunk::ucrt
{

  using namespace __crt_strtox;

  namespace
  {

    class fp_control_word_guard
    {
    public:
      explicit fp_control_word_guard(unsigned int const mask = ~0u)
          : _mask(mask)
      {
        _controlfp_s(&_original_control_word, 0, 0);
      }

      fp_control_word_guard(unsigned int const new_control,
                            unsigned int const mask)
          : _mask(mask)
      {
        unsigned int float_control;
        _controlfp_s(&_original_control_word, 0, 0);
        _controlfp_s(&float_control, new_control, _mask);
      }

      ~fp_control_word_guard()
      {
        unsigned int reset_cw;
        _controlfp_s(&reset_cw, _original_control_word, _mask);
      }

    private:
      unsigned int _original_control_word;
      unsigned int _mask;
    };

    class scoped_fp_state_reset
    {
    public:
      scoped_fp_state_reset() throw()
      {
        fegetenv(&_environment);
        if ((_environment._Fe_ctl & FE_ALL_EXCEPT) == FE_ALL_EXCEPT)
        {
          _requires_reset = false;
        }
        else
        {
          feholdexcept(&_environment);
          _requires_reset = true;
        }
      }

      ~scoped_fp_state_reset() throw()
      {
        if (_requires_reset)
        {
          fesetenv(&_environment);
        }
      }

    private:
      scoped_fp_state_reset(scoped_fp_state_reset const &);
      scoped_fp_state_reset &operator=(scoped_fp_state_reset const &);

      fenv_t _environment;
      bool _requires_reset;
    };

  } // namespace

  template <typename FloatingType>
  __forceinline static __acrt_has_trailing_digits __cdecl
  convert_to_fos_high_precision(FloatingType const value,
                                uint32_t const precision,
                                __acrt_precision_style const precision_style,
                                int *const exponent,
                                char *const mantissa_buffer,
                                size_t const mantissa_buffer_count) throw()
  {
    using floating_traits = __acrt_floating_type_traits<FloatingType>;
    using components_type = typename floating_traits::components_type;

    _ASSERTE(mantissa_buffer_count > 0);

    components_type const &value_components =
        reinterpret_cast<components_type const &>(value);

    bool const is_denormal = value_components._exponent == 0;

    uint64_t const mantissa_adjustment =
        is_denormal
            ? 0
            : static_cast<uint64_t>(1) << (floating_traits::mantissa_bits - 1);

    int32_t const exponent_adjustment = is_denormal ? 2 : 1;

    uint64_t const f = value_components._mantissa + mantissa_adjustment;
    int32_t const e = static_cast<int32_t>(value_components._exponent) -
                      floating_traits::exponent_bias -
                      floating_traits::mantissa_bits + exponent_adjustment;

    int32_t k = static_cast<int32_t>(ceil(log10(value)));
    if (k == INT32_MAX || k == INT32_MIN)
    {
      _ASSERTE(("unexpected input value; log10 failed", 0));
      k = 0;
    }

    big_integer r = make_big_integer(f);
    big_integer s{};

    if (e >= 0)
    {
      if (r !=
          make_big_integer_power_of_two(floating_traits::mantissa_bits - 1))
      {
        shift_left(r, e + 1);
        s = make_big_integer(2);
      }
      else
      {
        shift_left(r, e + 2);
        s = make_big_integer(4);
      }
    }
    else
    {
      if (e == floating_traits::minimum_binary_exponent ||
          r !=
              make_big_integer_power_of_two(floating_traits::mantissa_bits - 1))
      {
        shift_left(r, 1);
        s = make_big_integer_power_of_two(-e + 1);
      }
      else
      {
        shift_left(r, 2);
        s = make_big_integer_power_of_two(-e + 2);
      }
    }

    if (k >= 0)
    {
      multiply_by_power_of_ten(s, k);
    }
    else
    {
      multiply_by_power_of_ten(r, -k);
    }

    char *mantissa_it = mantissa_buffer;

    multiply(r, 10);
    uint32_t const initial_digit = static_cast<uint32_t>(divide(r, s));

    if (initial_digit == 10)
    {
      ++k;
      *mantissa_it++ = '1';
      multiply(s, 10);
    }
    else if (initial_digit == 0)
    {
      --k;
    }
    else
    {
      *mantissa_it++ = static_cast<char>('0' + initial_digit);
    }

    *exponent = k;

    uint32_t const required_digits =
        k >= 0 && precision <= INT_MAX &&
                precision_style == __acrt_precision_style::fixed
            ? k + precision
            : precision;

    char *const mantissa_last =
        mantissa_buffer + __min(mantissa_buffer_count - 1, required_digits);

    bool unwritten_nonzero_digits_in_chunk = false;
    for (;;)
    {
      if (mantissa_it == mantissa_last)
      {
        break;
      }

      if (is_zero(r))
      {
        break;
      }

      uint32_t const digits_per_iteration = 9;
      uint32_t const digits_per_iteration_multiplier = 1000 * 1000 * 1000;

      multiply(r, digits_per_iteration_multiplier);
      uint32_t quotient = static_cast<uint32_t>(divide(r, s));

      _ASSERTE(quotient < digits_per_iteration_multiplier);

      for (uint32_t i = digits_per_iteration - 1;
           i != static_cast<uint32_t>(-1);
           --i)
      {
        char const d = static_cast<char>('0' + quotient % 10);
        quotient /= 10;

        if (static_cast<uint32_t>(mantissa_last - mantissa_it) <= i)
        {
          if (d != '0')
          {
            unwritten_nonzero_digits_in_chunk = true;
          }

          continue;
        }

        mantissa_it[i] = d;
      }

      mantissa_it += __min(digits_per_iteration, mantissa_last - mantissa_it);
    }

    *mantissa_it = '\0';

    bool const all_zeros_after_chunk = is_zero(r);

    if (all_zeros_after_chunk && !unwritten_nonzero_digits_in_chunk)
    {
      return __acrt_has_trailing_digits::no_trailing;
    }

    return __acrt_has_trailing_digits::trailing;
  }

  __acrt_has_trailing_digits __cdecl
  __acrt_fltout(_CRT_DOUBLE value,
                unsigned const precision,
                __acrt_precision_style const precision_style,
                STRFLT const flt,
                char *const result,
                size_t const result_count)
  {
    using floating_traits = __acrt_floating_type_traits<double>;
    using components_type = floating_traits::components_type;

    scoped_fp_state_reset const reset_fp_state;

    components_type &components = reinterpret_cast<components_type &>(value);

    flt->sign = components._sign == 1 ? '-' : ' ';
    flt->mantissa = result;

    unsigned int float_control;
    _controlfp_s(&float_control, 0, 0);
    bool const value_is_zero =
        components._exponent == 0 &&
        (components._mantissa == 0 || float_control & _DN_FLUSH);
    if (value_is_zero)
    {
      flt->decpt = 0;
      _ERRCHECK(strcpy_s(result, result_count, "0"));
      return __acrt_has_trailing_digits::no_trailing;
    }

    __acrt_fp_class const classification = __acrt_fp_classify(value.x);
    if (classification != __acrt_fp_class::finite)
    {
      flt->decpt = 1;
    }

    switch (classification)
    {
    case __acrt_fp_class::infinity:
      _ERRCHECK(strcpy_s(result, result_count, "1#INF"));
      return __acrt_has_trailing_digits::trailing;
    case __acrt_fp_class::quiet_nan:
      _ERRCHECK(strcpy_s(result, result_count, "1#QNAN"));
      return __acrt_has_trailing_digits::no_trailing;
    case __acrt_fp_class::signaling_nan:
      _ERRCHECK(strcpy_s(result, result_count, "1#SNAN"));
      return __acrt_has_trailing_digits::no_trailing;
    case __acrt_fp_class::indeterminate:
      _ERRCHECK(strcpy_s(result, result_count, "1#IND"));
      return __acrt_has_trailing_digits::no_trailing;
    }

    components._sign = 0;

    fp_control_word_guard const fpc(_MCW_EM, _MCW_EM);
    return convert_to_fos_high_precision(value.x,
                                         precision + 1,
                                         precision_style,
                                         &flt->decpt,
                                         result,
                                         result_count);
  }

} // namespace mingw_thunk::ucrt
