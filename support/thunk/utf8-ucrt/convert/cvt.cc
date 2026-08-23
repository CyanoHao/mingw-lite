#include "../inc/corecrt_internal_fltintrn.h"
#include "../inc/corecrt_internal_ptd_propagation.h"

#include <fenv.h>
#include <stdio.h>
#include <string.h>

#include <crtdbg.h>

namespace mingw_thunk::ucrt
{

  static void __cdecl shift_bytes(
      _Maybe_unsafe_(_Inout_updates_z_, buffer_count) char *const buffer_base,
      _In_ size_t const buffer_count,
      _In_range_(buffer_base, buffer_base + buffer_count) char *const string,
      _In_ int const distance) throw()
  {
    if (distance != 0)
    {
      memmove(string + distance, string, strlen(string) + 1);
    }
  }

  _Success_(return == 0) static errno_t __cdecl fp_format_nan_or_infinity(
      _In_ __acrt_fp_class const classification,
      _In_range_(0, 1) bool const is_negative,
      _Maybe_unsafe_(_Out_writes_z_, result_buffer_count) char *result_buffer,
      _In_range_(1, SIZE_MAX) size_t result_buffer_count,
      _In_range_(0, 1) bool const use_capitals) noexcept
  {
    using floating_traits = __acrt_floating_type_traits<double>;
    using components_type = floating_traits::components_type;

    if (result_buffer_count < _countof("INF") + is_negative)
    {
      *result_buffer = '\0';
      return ENOMEM;
    }

    if (is_negative)
    {
      *result_buffer++ = '-';
      *result_buffer = '\0';
      if (result_buffer_count != _CRT_UNBOUNDED_BUFFER_SIZE)
      {
        --result_buffer_count;
      }
    }

    static char const *const strings[][4] = {
        {"INF", "INF", "inf", "inf"},
        {"NAN", "NAN", "nan", "nan"},
        {"NAN(SNAN)", "NAN", "nan(snan)", "nan"},
        {"NAN(IND)", "NAN", "nan(ind)", "nan"},
    };

    uint32_t const row = static_cast<uint32_t>(classification) - 1;
    uint32_t const column = use_capitals ? 0 : 2;

    bool const long_string_will_fit =
        result_buffer_count > strlen(strings[row][column]);
    _ERRCHECK(strcpy_s(result_buffer,
                       result_buffer_count,
                       strings[row][column + !long_string_will_fit]));
    return 0;
  }

  _Success_(return == 0) static errno_t fp_format_e_internal(
      _Maybe_unsafe_(_Inout_updates_z_,
                     result_buffer_count) char *const result_buffer,
      _In_fits_precision_(precision) size_t const result_buffer_count,
      _In_ int const precision,
      _In_ bool const capitals,
      _In_ unsigned const min_exponent_digits,
      _In_ STRFLT const pflt,
      _In_ bool const g_fmt,
      _Inout_ __crt_cached_ptd_host &ptd) throw()
  {
    _UCRT_VALIDATE_RETURN_ERRCODE(
        ptd,
        result_buffer_count >
            static_cast<size_t>(3 + (precision > 0 ? precision : 0) + 5 + 1),
        ERANGE);

    if (g_fmt)
    {
      char *const p = result_buffer + (pflt->sign == '-');
      shift_bytes(result_buffer, result_buffer_count, p, precision > 0);
    }

    char *p = result_buffer;

    if (pflt->sign == '-')
      *p++ = '-';

    if (precision > 0)
    {
      *p = *(p + 1);
      *++p = '.';
    }

    p = p + precision + (g_fmt ? 0 : 1);
    _ERRCHECK(strcpy_s(p,
                       result_buffer_count == _CRT_UNBOUNDED_BUFFER_SIZE
                           ? result_buffer_count
                           : result_buffer_count - (p - result_buffer),
                       "e+000"));
    char *exponentpos = p + 2;

    if (capitals)
      *p = 'E';

    ++p;

    if (*pflt->mantissa != '0')
    {
      int exp = pflt->decpt - 1;
      if (exp < 0)
      {
        exp = -exp;
        *p = '-';
      }

      ++p;

      if (exp >= 100)
      {
        *p += static_cast<char>(exp / 100);
        exp %= 100;
      }

      ++p;

      if (exp >= 10)
      {
        *p += static_cast<char>(exp / 10);
        exp %= 10;
      }

      *++p += static_cast<char>(exp);
    }

    if (min_exponent_digits == 2)
    {
      if (*exponentpos == '0')
      {
        memmove(exponentpos, exponentpos + 1, 3);
      }
    }

    return 0;
  }

  _Success_(return == 0) static errno_t __cdecl
  fp_format_e(_In_ double const *const argument,
              _Maybe_unsafe_(_Inout_updates_z_,
                             result_buffer_count) char *const result_buffer,
              _In_fits_precision_(precision) size_t const result_buffer_count,
              _Out_writes_(scratch_buffer_count) char *const scratch_buffer,
              _In_ size_t const scratch_buffer_count,
              _In_ int const precision,
              _In_ bool const capitals,
              _In_ unsigned const min_exponent_digits,
              _In_ __acrt_rounding_mode const rounding_mode,
              _Inout_ __crt_cached_ptd_host &ptd) throw()
  {
    _strflt strflt;

    __acrt_has_trailing_digits const trailing_digits =
        __acrt_fltout(*reinterpret_cast<_CRT_DOUBLE const *>(argument),
                      precision + 1,
                      __acrt_precision_style::scientific,
                      &strflt,
                      scratch_buffer,
                      scratch_buffer_count);

    errno_t const e = __acrt_fp_strflt_to_string(
        result_buffer + (strflt.sign == '-') + (precision > 0),
        (result_buffer_count == _CRT_UNBOUNDED_BUFFER_SIZE
             ? result_buffer_count
             : result_buffer_count - (strflt.sign == '-') - (precision > 0)),
        precision + 1,
        &strflt,
        trailing_digits,
        rounding_mode,
        ptd);

    if (e != 0)
    {
      result_buffer[0] = '\0';
      return e;
    }

    return fp_format_e_internal(result_buffer,
                                result_buffer_count,
                                precision,
                                capitals,
                                min_exponent_digits,
                                &strflt,
                                false,
                                ptd);
  }

  static bool fe_to_nearest(double const *const argument,
                            unsigned __int64 const mask,
                            short const maskpos)
  {
    using floating_traits = __acrt_floating_type_traits<double>;
    using components_type = floating_traits::components_type;
    components_type const *const components =
        reinterpret_cast<components_type const *>(argument);

    unsigned short digit =
        static_cast<unsigned short>((components->_mantissa & mask) >> maskpos);

    if (digit > 8)
    {
      return true;
    }

    if (digit < 8)
    {
      return false;
    }

    unsigned __int64 const roundBitsMask =
        (static_cast<unsigned __int64>(1) << maskpos) - 1;
    if (components->_mantissa & roundBitsMask)
    {
      return true;
    }

    if (maskpos != DBL_MANT_DIG - 5)
    {
      digit = static_cast<unsigned short>(
          ((components->_mantissa / 16) & mask) >> maskpos);
    }
    else
    {
      digit = components->_exponent == 0 ? 0 : 1;
    }

    return digit % 2 == 1;
  }

  static bool should_round_up(double const *const argument,
                              unsigned __int64 const mask,
                              short const maskpos,
                              __acrt_rounding_mode const rounding_mode)
  {
    using floating_traits = __acrt_floating_type_traits<double>;
    using components_type = floating_traits::components_type;
    components_type const *const components =
        reinterpret_cast<components_type const *>(argument);

    unsigned short const digit =
        static_cast<unsigned short>((components->_mantissa & mask) >> maskpos);

    if (rounding_mode == __acrt_rounding_mode::legacy)
    {
      return digit >= 8;
    }
    int const round_mode = fegetround();

    if (round_mode == FE_TONEAREST)
    {
      return fe_to_nearest(argument, mask, maskpos);
    }

    if (round_mode == FE_UPWARD)
    {
      return digit != 0 && !components->_sign;
    }

    if (round_mode == FE_DOWNWARD)
    {
      return digit != 0 && components->_sign;
    }

    return false;
  }

  _Success_(return == 0) static errno_t __cdecl
  fp_format_a(_In_ double const *const argument,
              _Maybe_unsafe_(_Inout_updates_z_,
                             result_buffer_count) char *result_buffer,
              _In_fits_precision_(precision) size_t const result_buffer_count,
              _Out_writes_(scratch_buffer_count) char *const scratch_buffer,
              _In_ size_t const scratch_buffer_count,
              _In_ int precision,
              _In_ bool const capitals,
              _In_ unsigned const min_exponent_digits,
              _In_ __acrt_rounding_mode const rounding_mode,
              _Inout_ __crt_cached_ptd_host &ptd)
  {
    using floating_traits = __acrt_floating_type_traits<double>;
    using components_type = floating_traits::components_type;

    if (precision < 0)
    {
      precision = 0;
    }

    result_buffer[0] = '\0';

    _UCRT_VALIDATE_RETURN_ERRCODE(
        ptd,
        result_buffer_count > static_cast<size_t>(1 + 4 + precision + 6),
        ERANGE);

    components_type const *const components =
        reinterpret_cast<components_type const *>(argument);
    if (components->_exponent == floating_traits::exponent_mask)
    {
      errno_t const e = fp_format_e(argument,
                                    result_buffer,
                                    result_buffer_count,
                                    scratch_buffer,
                                    scratch_buffer_count,
                                    precision,
                                    false,
                                    min_exponent_digits,
                                    rounding_mode,
                                    ptd);

      if (e != 0)
      {
        result_buffer[0] = '\0';
        return e;
      }

      char *p = strrchr(result_buffer, 'e');
      if (p)
      {
        *p = capitals ? 'P' : 'p';

        p += 3;
        *p = 0;
      }
      return e;
    }

    if (components->_sign)
    {
      *result_buffer++ = '-';
    }

    int const hexadd = (capitals ? 'A' : 'a') - '9' - 1;

    unsigned __int64 debias = floating_traits::exponent_bias;
    if (components->_exponent == 0)
    {
      *result_buffer++ = '0';
      if (components->_mantissa == 0)
      {
        debias = 0;
      }
      else
      {
        debias--;
      }
    }
    else
    {
      *result_buffer++ = '1';
    }

    char *pos = result_buffer++;
    if (precision == 0)
    {
      *pos = 0;
    }
    else
    {
      *pos = '.';
    }

    if (components->_mantissa > 0)
    {
      short maskpos = (floating_traits::mantissa_bits - 1) - 4;

      unsigned __int64 mask = 0xf;
      mask <<= maskpos;

      while (maskpos >= 0 && precision > 0)
      {
        unsigned short digit = static_cast<unsigned short>(
            (components->_mantissa & mask) >> maskpos);
        digit += '0';
        if (digit > '9')
        {
          digit += static_cast<unsigned short>(hexadd);
        }
        *result_buffer++ = static_cast<char>(digit);
        mask >>= 4;
        maskpos -= 4;
        --precision;
      }

      if (maskpos >= 0)
      {
        if (should_round_up(argument, mask, maskpos, rounding_mode))
        {
          char *p = result_buffer;
          --p;
          while (*p == 'f' || *p == 'F')
          {
            *p-- = '0';
          }
          if (p != pos)
          {
            if (*p == '9')
            {
              *p += static_cast<char>(1 + hexadd);
            }
            else
            {
              *p += 1;
            }
          }
          else
          {
            --p;

            *p += 1;
          }
        }
      }
    }

    for (; precision > 0; --precision)
    {
      *result_buffer++ = '0';
    }

    if (*pos == 0)
    {
      result_buffer = pos;
    }

    *result_buffer++ = capitals ? 'P' : 'p';
    __int64 exponent = components->_exponent - debias;
    if (exponent >= 0)
    {
      *result_buffer++ = '+';
    }
    else
    {
      *result_buffer++ = '-';
      exponent = -exponent;
    }
    pos = result_buffer;
    *pos = '0';

    if (exponent >= 1000)
    {
      *result_buffer++ = '0' + static_cast<char>(exponent / 1000);
      exponent %= 1000;
    }
    if (result_buffer != pos || exponent >= 100)
    {
      *result_buffer++ = '0' + static_cast<char>(exponent / 100);
      exponent %= 100;
    }
    if (result_buffer != pos || exponent >= 10)
    {
      *result_buffer++ = '0' + static_cast<char>(exponent / 10);
      exponent %= 10;
    }

    *result_buffer++ = '0' + static_cast<char>(exponent);

    *result_buffer = '\0';

    return 0;
  }

  _Success_(return == 0) static errno_t fp_format_f_internal(
      _Pre_z_ _Maybe_unsafe_(_Inout_updates_z_,
                             buffer_count) char *const buffer,
      _In_fits_precision_(precision) size_t const buffer_count,
      _In_ int const precision,
      _In_ STRFLT const pflt,
      _In_ bool const g_fmt,
      _Inout_ __crt_cached_ptd_host &ptd) throw()
  {
    int const g_magnitude = pflt->decpt - 1;

    if (g_fmt && g_magnitude == precision)
    {
      char *const p = g_magnitude + buffer + (pflt->sign == '-');
      p[0] = '0';
      p[1] = '\0';
    }

    char *p = buffer;

    if (pflt->sign == '-')
      *p++ = '-';

    if (pflt->decpt <= 0)
    {
      bool const is_zero_pflt = pflt->decpt == 0 && *pflt->mantissa == '0';
      if (!g_fmt || !is_zero_pflt)
      {
        shift_bytes(buffer, buffer_count, p, 1);
      }
      *p++ = '0';
    }
    else
    {
      p += pflt->decpt;
    }

    if (precision > 0)
    {
      shift_bytes(buffer, buffer_count, p, 1);
      *p++ = '.';

      if (pflt->decpt < 0)
      {
        int const computed_precision =
            (g_fmt || -pflt->decpt < precision) ? -pflt->decpt : precision;

        shift_bytes(buffer, buffer_count, p, computed_precision);
        memset(p, '0', computed_precision);
      }
    }

    return 0;
  }

  _Success_(return == 0) static errno_t __cdecl
  fp_format_f(_In_ double const *const argument,
              _Maybe_unsafe_(_Inout_updates_z_,
                             result_buffer_count) char *const result_buffer,
              _In_fits_precision_(precision) size_t const result_buffer_count,
              _Out_writes_(scratch_buffer_count) char *const scratch_buffer,
              _In_ size_t const scratch_buffer_count,
              _In_ int const precision,
              _In_ __acrt_rounding_mode const rounding_mode,
              _Inout_ __crt_cached_ptd_host &ptd) throw()
  {
    _strflt strflt{};
    __acrt_has_trailing_digits const trailing_digits =
        __acrt_fltout(*reinterpret_cast<_CRT_DOUBLE const *>(argument),
                      precision,
                      __acrt_precision_style::fixed,
                      &strflt,
                      scratch_buffer,
                      scratch_buffer_count);

    errno_t const e = __acrt_fp_strflt_to_string(
        result_buffer + (strflt.sign == '-'),
        (result_buffer_count == _CRT_UNBOUNDED_BUFFER_SIZE
             ? result_buffer_count
             : result_buffer_count - (strflt.sign == '-')),
        precision + strflt.decpt,
        &strflt,
        trailing_digits,
        rounding_mode,
        ptd);

    if (e != 0)
    {
      result_buffer[0] = '\0';
      return e;
    }

    return fp_format_f_internal(
        result_buffer, result_buffer_count, precision, &strflt, false, ptd);
  }

  _Success_(return == 0) static errno_t __cdecl
  fp_format_g(_In_ double const *const argument,
              _Maybe_unsafe_(_Inout_updates_z_,
                             result_buffer_count) char *const result_buffer,
              _In_fits_precision_(precision) size_t const result_buffer_count,
              _Out_writes_(scratch_buffer_count) char *const scratch_buffer,
              _In_ size_t const scratch_buffer_count,
              _In_ int const precision,
              _In_ bool const capitals,
              _In_ unsigned const min_exponent_digits,
              _In_ __acrt_rounding_mode const rounding_mode,
              _Inout_ __crt_cached_ptd_host &ptd) throw()
  {
    _strflt strflt{};

    __acrt_has_trailing_digits const trailing_digits =
        __acrt_fltout(*reinterpret_cast<_CRT_DOUBLE const *>(argument),
                      precision,
                      __acrt_precision_style::fixed,
                      &strflt,
                      scratch_buffer,
                      scratch_buffer_count);

    size_t const minus_sign_length = strflt.sign == '-' ? 1 : 0;

    int g_magnitude = strflt.decpt - 1;
    char *p = result_buffer + minus_sign_length;

    size_t const buffer_count_for_fptostr =
        result_buffer_count == _CRT_UNBOUNDED_BUFFER_SIZE
            ? result_buffer_count
            : result_buffer_count - minus_sign_length;

    errno_t const fptostr_result =
        __acrt_fp_strflt_to_string(p,
                                   buffer_count_for_fptostr,
                                   precision,
                                   &strflt,
                                   trailing_digits,
                                   rounding_mode,
                                   ptd);
    if (fptostr_result != 0)
    {
      result_buffer[0] = '\0';
      return fptostr_result;
    }

    bool const g_round_expansion = g_magnitude < (strflt.decpt - 1);

    g_magnitude = strflt.decpt - 1;

    if (g_magnitude < -4 || g_magnitude >= precision)
    {
      return fp_format_e_internal(result_buffer,
                                  result_buffer_count,
                                  precision,
                                  capitals,
                                  min_exponent_digits,
                                  &strflt,
                                  true,
                                  ptd);
    }
    else
    {
      if (g_round_expansion)
      {
        while (*p++)
        {
        }

        *(p - 2) = '\0';
      }

      return fp_format_f_internal(
          result_buffer, result_buffer_count, precision, &strflt, true, ptd);
    }
  }

  errno_t __cdecl __acrt_fp_format(double const *const value,
                                   char *const result_buffer,
                                   size_t const result_buffer_count,
                                   char *const scratch_buffer,
                                   size_t const scratch_buffer_count,
                                   int const format,
                                   int const precision,
                                   uint64_t const options,
                                   __acrt_rounding_mode rounding_mode,
                                   __crt_cached_ptd_host &ptd)
  {
    _UCRT_VALIDATE_RETURN_ERRCODE(ptd, result_buffer != nullptr, EINVAL);
    _UCRT_VALIDATE_RETURN_ERRCODE(ptd, result_buffer_count > 0, EINVAL);
    _UCRT_VALIDATE_RETURN_ERRCODE(ptd, scratch_buffer != nullptr, EINVAL);
    _UCRT_VALIDATE_RETURN_ERRCODE(ptd, scratch_buffer_count > 0, EINVAL);

    bool const use_capitals =
        format == 'A' || format == 'E' || format == 'F' || format == 'G';

    if ((options & _CRT_INTERNAL_PRINTF_LEGACY_MSVCRT_COMPATIBILITY) == 0)
    {
      __acrt_fp_class const classification = __acrt_fp_classify(*value);
      if (classification != __acrt_fp_class::finite)
      {
        return fp_format_nan_or_infinity(classification,
                                         __acrt_fp_is_negative(*value),
                                         result_buffer,
                                         result_buffer_count,
                                         use_capitals);
      }
    }

    unsigned const min_exponent_digits =
        (options & _CRT_INTERNAL_PRINTF_LEGACY_THREE_DIGIT_EXPONENTS) != 0 ? 3
                                                                           : 2;
    if ((options & _CRT_INTERNAL_PRINTF_STANDARD_ROUNDING) == 0)
    {
      rounding_mode = __acrt_rounding_mode::legacy;
    }

    switch (format)
    {
    case 'a':
    case 'A':
      return fp_format_a(value,
                         result_buffer,
                         result_buffer_count,
                         scratch_buffer,
                         scratch_buffer_count,
                         precision,
                         use_capitals,
                         min_exponent_digits,
                         rounding_mode,
                         ptd);

    case 'e':
    case 'E':
      return fp_format_e(value,
                         result_buffer,
                         result_buffer_count,
                         scratch_buffer,
                         scratch_buffer_count,
                         precision,
                         use_capitals,
                         min_exponent_digits,
                         rounding_mode,
                         ptd);

    case 'f':
    case 'F':
      return fp_format_f(value,
                         result_buffer,
                         result_buffer_count,
                         scratch_buffer,
                         scratch_buffer_count,
                         precision,
                         rounding_mode,
                         ptd);

    default:
      _ASSERTE(("Unsupported format specifier", 0));
    case 'g':
    case 'G':
      return fp_format_g(value,
                         result_buffer,
                         result_buffer_count,
                         scratch_buffer,
                         scratch_buffer_count,
                         precision,
                         use_capitals,
                         min_exponent_digits,
                         rounding_mode,
                         ptd);
    }
  }

} // namespace mingw_thunk::ucrt
