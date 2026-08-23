#include "../inc/corecrt_internal_fltintrn.h"
#include "../inc/corecrt_internal_ptd_propagation.h"

#include <fenv.h>
#include <string.h>

namespace mingw_thunk::ucrt
{

  static bool check_trailing(char const *mantissa_it,
                             __acrt_has_trailing_digits const trailing_digits)
  {
    if (trailing_digits == __acrt_has_trailing_digits::trailing)
    {
      return true;
    }

    while (*mantissa_it == '0')
    {
      mantissa_it++;
    }

    if (*mantissa_it != '\0')
    {
      return true;
    }

    return false;
  }

  static bool should_round_up(char const *const mantissa_base,
                              char const *const mantissa_it,
                              int const sign,
                              __acrt_has_trailing_digits const trailing_digits,
                              __acrt_rounding_mode const rounding_mode)
  {
    if (rounding_mode == __acrt_rounding_mode::legacy)
    {
      return *mantissa_it >= '5';
    }

    int const round_mode = fegetround();

    if (round_mode == FE_TONEAREST)
    {
      if (*mantissa_it > '5')
      {
        return true;
      }

      if (*mantissa_it < '5')
      {
        return false;
      }

      if (check_trailing(mantissa_it + 1, trailing_digits))
      {
        return true;
      }

      if (mantissa_it == mantissa_base)
      {
        return false;
      }

      return *(mantissa_it - 1) % 2;
    }

    if (round_mode == FE_UPWARD)
    {
      return check_trailing(mantissa_it, trailing_digits) && sign != '-';
    }

    if (round_mode == FE_DOWNWARD)
    {
      return check_trailing(mantissa_it, trailing_digits) && sign == '-';
    }

    return false;
  }

  errno_t __cdecl
  __acrt_fp_strflt_to_string(char *const buffer,
                             size_t const buffer_count,
                             int digits,
                             STRFLT const pflt,
                             __acrt_has_trailing_digits const trailing_digits,
                             __acrt_rounding_mode const rounding_mode,
                             __crt_cached_ptd_host &ptd)
  {
    _UCRT_VALIDATE_RETURN_ERRCODE(ptd, buffer != nullptr, EINVAL);
    _UCRT_VALIDATE_RETURN_ERRCODE(ptd, buffer_count > 0, EINVAL);
    buffer[0] = '\0';

    _UCRT_VALIDATE_RETURN_ERRCODE(
        ptd,
        buffer_count > static_cast<size_t>((digits > 0 ? digits : 0) + 1),
        ERANGE);
    _UCRT_VALIDATE_RETURN_ERRCODE(ptd, pflt != nullptr, EINVAL);

    char *buffer_it = buffer;
    char *const mantissa_base = pflt->mantissa;
    char *mantissa_it = pflt->mantissa;

    *buffer_it++ = '0';

    while (digits > 0)
    {
      *buffer_it++ = *mantissa_it ? *mantissa_it++ : '0';
      --digits;
    }

    *buffer_it = '\0';

    if (digits >= 0 && should_round_up(mantissa_base,
                                       mantissa_it,
                                       pflt->sign,
                                       trailing_digits,
                                       rounding_mode))
    {
      buffer_it--;

      while (*buffer_it == '9')
      {
        *buffer_it-- = '0';
      }

      *buffer_it += 1;
    }

    if (*buffer == '1')
    {
      pflt->decpt++;
    }
    else
    {
      memmove(buffer, buffer + 1, strlen(buffer + 1) + 1);
    }

    return 0;
  }

} // namespace mingw_thunk::ucrt
