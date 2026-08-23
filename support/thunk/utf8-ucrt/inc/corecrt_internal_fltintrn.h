#pragma once

#include "corecrt_internal.h"

#include <errno.h>
#include <float.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include <sal.h>

namespace mingw_thunk::ucrt
{

  template <typename FloatingType>
  struct __acrt_floating_type_traits;

  template <>
  struct __acrt_floating_type_traits<float>
  {
    enum : int32_t
    {
      mantissa_bits = FLT_MANT_DIG,
      exponent_bits = sizeof(float) * CHAR_BIT - FLT_MANT_DIG,

      maximum_binary_exponent = FLT_MAX_EXP - 1,
      minimum_binary_exponent = FLT_MIN_EXP - 1,

      exponent_bias = 127
    };

    enum : uint32_t
    {
      exponent_mask = (1u << (exponent_bits)) - 1,
      normal_mantissa_mask = (1u << (mantissa_bits)) - 1,
      denormal_mantissa_mask = (1u << (mantissa_bits - 1)) - 1,

      special_nan_mantissa_mask = (1u << (mantissa_bits - 2))
    };

    struct components_type
    {
      uint32_t _mantissa : mantissa_bits - 1;
      uint32_t _exponent : exponent_bits;
      uint32_t _sign : 1;
    };

    static_assert(sizeof(components_type) == sizeof(float),
                  "unexpected components size");
  };

  template <>
  struct __acrt_floating_type_traits<double>
  {
    enum : int32_t
    {
      mantissa_bits = DBL_MANT_DIG,
      exponent_bits = sizeof(double) * CHAR_BIT - DBL_MANT_DIG,

      maximum_binary_exponent = DBL_MAX_EXP - 1,
      minimum_binary_exponent = DBL_MIN_EXP - 1,

      exponent_bias = 1023
    };

    enum : uint64_t
    {
      exponent_mask = (uint64_t(1) << (exponent_bits)) - 1,
      normal_mantissa_mask = (uint64_t(1) << (mantissa_bits)) - 1,
      denormal_mantissa_mask = (uint64_t(1) << (mantissa_bits - 1)) - 1,

      special_nan_mantissa_mask = (uint64_t(1) << (mantissa_bits - 2))
    };

    struct components_type
    {
      uint64_t _mantissa : mantissa_bits - 1;
      uint64_t _exponent : exponent_bits;
      uint64_t _sign : 1;
    };

    static_assert(sizeof(components_type) == sizeof(double),
                  "unexpected components size");
  };

  enum class __acrt_fp_class : uint32_t
  {
    finite,
    infinity,
    quiet_nan,
    signaling_nan,
    indeterminate,
  };

  enum class __acrt_has_trailing_digits
  {
    trailing,
    no_trailing
  };

  enum class __acrt_precision_style
  {
    fixed,
    scientific
  };

  enum class __acrt_rounding_mode
  {
    legacy,
    standard
  };

  inline __acrt_fp_class __cdecl __acrt_fp_classify(double const &value) throw()
  {
    using floating_traits = __acrt_floating_type_traits<double>;
    using components_type = floating_traits::components_type;

    components_type const &components =
        reinterpret_cast<components_type const &>(value);

    bool const value_is_nan_or_infinity =
        components._exponent == (1u << floating_traits::exponent_bits) - 1;
    if (!value_is_nan_or_infinity)
    {
      return __acrt_fp_class::finite;
    }
    else if (components._mantissa == 0)
    {
      return __acrt_fp_class::infinity;
    }
    else if (components._sign == 1 &&
             components._mantissa == floating_traits::special_nan_mantissa_mask)
    {
      return __acrt_fp_class::indeterminate;
    }
    else if (components._mantissa & floating_traits::special_nan_mantissa_mask)
    {
      return __acrt_fp_class::quiet_nan;
    }
    else
    {
      return __acrt_fp_class::signaling_nan;
    }
  }

  inline bool __cdecl __acrt_fp_is_negative(double const &value) throw()
  {
    using floating_traits = __acrt_floating_type_traits<double>;
    using components_type = floating_traits::components_type;

    components_type const &components =
        reinterpret_cast<components_type const &>(value);

    return components._sign == 1;
  }

  struct _strflt
  {
    int sign;
    int decpt;
    char *mantissa;
  };

  typedef _strflt *STRFLT;

#define _In_fits_precision_(precision_arg)                                     \
  _When_(precision_arg <= 0, _Pre_satisfies_(_Curr_ > 9))                      \
      _When_(precision_arg > 0, _Pre_satisfies_(_Curr_ > 9 + precision_arg))

  _Success_(return == 0) errno_t __cdecl
  __acrt_fp_format(_In_ double const *value,
                   _Maybe_unsafe_(_Inout_updates_z_,
                                  result_buffer_count) char *result_buffer,
                   _In_fits_precision_(precision) size_t result_buffer_count,
                   _Out_writes_(scratch_buffer_count) char *scratch_buffer,
                   _In_ size_t scratch_buffer_count,
                   _In_ int format,
                   _In_ int precision,
                   _In_ uint64_t options,
                   _In_ __acrt_rounding_mode rounding_mode,
                   _Inout_ __crt_cached_ptd_host &ptd);

  errno_t __cdecl __acrt_fp_strflt_to_string(
      _Out_writes_z_(buffer_count) char *buffer,
      _When_((digits > 0), _In_ _Pre_satisfies_(buffer_count > digits + 1))
          _When_((digits <= 0), _In_ _Pre_satisfies_(buffer_count > 1))
              _In_ size_t buffer_count,
      _In_ int digits,
      _Inout_ STRFLT value,
      _In_ __acrt_has_trailing_digits trailing_digits,
      _In_ __acrt_rounding_mode rounding_mode,
      _Inout_ __crt_cached_ptd_host &ptd);

  __acrt_has_trailing_digits __cdecl
  __acrt_fltout(_In_ _CRT_DOUBLE value,
                _In_ unsigned precision,
                _In_ __acrt_precision_style precision_style,
                _Out_ STRFLT result,
                _Out_writes_z_(buffer_count) char *buffer,
                _In_ size_t buffer_count);

} // namespace mingw_thunk::ucrt
