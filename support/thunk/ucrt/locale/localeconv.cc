#include <thunk/_common.h>
#include <thunk/_no_thunk.h>

#include <limits.h>
#include <locale.h>

namespace mingw_thunk
{
  static char *dot = const_cast<char *>(".");
  static char *empty = const_cast<char *>("");
  static wchar_t *w_dot = const_cast<wchar_t *>(L".");
  static wchar_t *w_empty = const_cast<wchar_t *>(L"");

  const static struct lconv c_lconv = {
      .decimal_point = dot,
      .thousands_sep = empty,
      .grouping = empty,
      .int_curr_symbol = empty,
      .currency_symbol = empty,
      .mon_decimal_point = empty,
      .mon_thousands_sep = empty,
      .mon_grouping = empty,
      .positive_sign = empty,
      .negative_sign = empty,
      .int_frac_digits = CHAR_MAX,
      .frac_digits = CHAR_MAX,
      .p_cs_precedes = CHAR_MAX,
      .p_sep_by_space = CHAR_MAX,
      .n_cs_precedes = CHAR_MAX,
      .n_sep_by_space = CHAR_MAX,
      .p_sign_posn = CHAR_MAX,
      .n_sign_posn = CHAR_MAX,
      ._W_decimal_point = w_dot,
      ._W_thousands_sep = w_empty,
      ._W_int_curr_symbol = w_empty,
      ._W_currency_symbol = w_empty,
      ._W_mon_decimal_point = w_empty,
      ._W_mon_thousands_sep = w_empty,
      ._W_positive_sign = w_empty,
      ._W_negative_sign = w_empty,
  };

  __DEFINE_THUNK(api_ms_win_crt_locale_l1_1_0,
                 0,
                 struct lconv *,
                 __cdecl,
                 localeconv,
                 void)
  {
    return const_cast<lconv *>(&c_lconv);
  }
} // namespace mingw_thunk
