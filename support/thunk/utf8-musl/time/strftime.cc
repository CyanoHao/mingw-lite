/* musl src/time/strftime.c port (plan-2 M9 §4.3).  Adaptations:
 *  - langinfo/__nl_langinfo_l replaced by the hardcoded C.UTF-8
 *    English tables (musl C locale values; wine anchor 2026-09-27:
 *    the native C-locale output is byte-identical for
 *    %a/%A/%b/%B/%p/%c/%x/%X — native only localizes after an
 *    LC_TIME switch, ours stays English by design)
 *  - the Windows struct tm has no __tm_gmtoff/__tm_tzname: the %z/%Z
 *    specifiers read the tz state, %s keeps the musl formula
 *    __tm_to_secs(tm) - gmtoff with the offset derived from the tz
 *    state (dst-aware)
 *  - snprintf comes from this layer's own vsnprintf engine; isdigit
 *    and strtoul are hand-rolled (no CRT ctype/stdlib coupling from
 *    the engine)
 *  - locale_t dropped: the tables are fixed English
 * musl quirks kept verbatim: the '*p' position used by the '+'-flag
 * decision (the post-width scan point BEFORE the E/O skip), the
 * partial-length truncation contract, and the epilogue NUL write. */

#include "../internal/time_impl.h"

#include <thunk/u8crt/musl.h>

#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>

namespace mingw_thunk
{
  namespace musl
  {
    namespace
    {
      int is_leap(int y)
      {
        /* Avoid overflow */
        if (y > INT_MAX - 1900)
          y -= 2000;
        y += 1900;
        return !(y % 4) && ((y % 100) || !(y % 400));
      }

      int week_num(const struct tm *tm)
      {
        int val = (int)((tm->tm_yday + 7U - (tm->tm_wday + 6U) % 7) / 7);
        /* If 1 Jan is just 1-3 days past Monday,
         * the previous week is also in this year. */
        if ((tm->tm_wday + 371U - tm->tm_yday - 2) % 7 <= 2)
          val++;
        if (!val) {
          val = 52;
          /* If 31 December of prev year a Thursday,
           * or Friday of a leap year, then the
           * prev year has 53 weeks. */
          int dec31 = (int)((tm->tm_wday + 7U - tm->tm_yday - 1) % 7);
          if (dec31 == 4 || (dec31 == 5 && is_leap(tm->tm_year % 400 - 1)))
            val++;
        } else if (val == 53) {
          /* If 1 January is not a Thursday, and not a
           * Wednesday of a leap year, then this
           * year has only 52 weeks. */
          int jan1 = (int)((tm->tm_wday + 371U - tm->tm_yday) % 7);
          if (jan1 != 4 && (jan1 != 3 || !is_leap(tm->tm_year)))
            val = 1;
        }
        return val;
      }

      /* C.UTF-8 English name tables (musl C locale nl_langinfo values) */
      const char *const k_abday[] = {"Sun", "Mon", "Tue", "Wed",
                                     "Thu", "Fri", "Sat"};
      const char *const k_day[] = {"Sunday",    "Monday",   "Tuesday",
                                   "Wednesday", "Thursday", "Friday",
                                   "Saturday"};
      const char *const k_abmon[] = {"Jan", "Feb", "Mar", "Apr",
                                     "May", "Jun", "Jul", "Aug",
                                     "Sep", "Oct", "Nov", "Dec"};
      const char *const k_mon[] = {"January",   "February", "March",
                                   "April",     "May",      "June",
                                   "July",      "August",   "September",
                                   "October",   "November", "December"};
      /* D_T_FMT / D_FMT / T_FMT / T_FMT_AMPM in the C locale */
      const char *const k_d_t_fmt = "%a %b %e %H:%M:%S %Y";
      const char *const k_d_fmt = "%m/%d/%y";
      const char *const k_t_fmt = "%H:%M:%S";
      const char *const k_t_fmt_ampm = "%I:%M:%S %p";

      inline bool is_digit(int c)
      {
        return c >= '0' && c <= '9';
      }

      int snprintf_wrap(char *s, size_t n, const char *fmt, ...)
      {
        va_list ap;
        va_start(ap, fmt);
        int r = vsnprintf(s, n, fmt, ap);
        va_end(ap);
        return r;
      }

      size_t strftime_engine(char *s, size_t n, const char *f,
                             const struct tm *tm);
    } // namespace

    const char *__strftime_fmt_1(char (*s)[100], size_t *l, int f,
                                 const struct tm *tm, int pad)
    {
      long long val;
      const char *fmt = "-";
      int width = 2, def_pad = '0';

      switch (f) {
      case 'a':
        if (tm->tm_wday > 6U)
          goto string;
        fmt = k_abday[tm->tm_wday];
        goto string;
      case 'A':
        if (tm->tm_wday > 6U)
          goto string;
        fmt = k_day[tm->tm_wday];
        goto string;
      case 'h':
      case 'b':
        if (tm->tm_mon > 11U)
          goto string;
        fmt = k_abmon[tm->tm_mon];
        goto string;
      case 'B':
        if (tm->tm_mon > 11U)
          goto string;
        fmt = k_mon[tm->tm_mon];
        goto string;
      case 'c':
        fmt = k_d_t_fmt;
        goto recu_strftime;
      case 'C':
        val = (1900LL + tm->tm_year) / 100;
        goto number;
      case 'e':
        def_pad = '_';
      case 'd':
        val = tm->tm_mday;
        goto number;
      case 'D':
        fmt = "%m/%d/%y";
        goto recu_strftime;
      case 'F':
        fmt = "%Y-%m-%d";
        goto recu_strftime;
      case 'g':
      case 'G':
        val = tm->tm_year + 1900LL;
        if (tm->tm_yday < 3 && week_num(tm) != 1)
          val--;
        else if (tm->tm_yday > 360 && week_num(tm) == 1)
          val++;
        if (f == 'g')
          val %= 100;
        else
          width = 4;
        goto number;
      case 'H':
        val = tm->tm_hour;
        goto number;
      case 'I':
        val = tm->tm_hour;
        if (!val)
          val = 12;
        else if (val > 12)
          val -= 12;
        goto number;
      case 'j':
        val = tm->tm_yday + 1;
        width = 3;
        goto number;
      case 'm':
        val = tm->tm_mon + 1;
        goto number;
      case 'M':
        val = tm->tm_min;
        goto number;
      case 'n':
        *l = 1;
        return "\n";
      case 'p':
        fmt = tm->tm_hour >= 12 ? "PM" : "AM";
        goto string;
      case 'r':
        fmt = k_t_fmt_ampm;
        goto recu_strftime;
      case 'R':
        fmt = "%H:%M";
        goto recu_strftime;
      case 's': {
        /* musl: val = __tm_to_secs(tm) - tm->__tm_gmtoff; the Windows
         * tm has no gmtoff — derive it from the tz state (dst-aware) */
        long off = tz::utc_offset(tm->tm_isdst > 0 ? 1 : 0);
        val = __tm_to_secs(tm) - off;
        width = 1;
        goto number;
      }
      case 'S':
        val = tm->tm_sec;
        goto number;
      case 't':
        *l = 1;
        return "\t";
      case 'T':
        fmt = "%H:%M:%S";
        goto recu_strftime;
      case 'u':
        val = tm->tm_wday ? tm->tm_wday : 7;
        width = 1;
        goto number;
      case 'U':
        val = (int)((tm->tm_yday + 7U - tm->tm_wday) / 7);
        goto number;
      case 'W':
        val = (int)((tm->tm_yday + 7U - (tm->tm_wday + 6U) % 7) / 7);
        goto number;
      case 'V':
        val = week_num(tm);
        goto number;
      case 'w':
        val = tm->tm_wday;
        width = 1;
        goto number;
      case 'x':
        fmt = k_d_fmt;
        goto recu_strftime;
      case 'X':
        fmt = k_t_fmt;
        goto recu_strftime;
      case 'y':
        val = (tm->tm_year + 1900LL) % 100;
        if (val < 0)
          val = -val;
        goto number;
      case 'Y':
        val = tm->tm_year + 1900LL;
        if (val >= 10000) {
          *l = (size_t)snprintf_wrap(*s, sizeof *s, "+%lld", val);
          return *s;
        }
        width = 4;
        goto number;
      case 'z': {
        if (tm->tm_isdst < 0) {
          *l = 0;
          return "";
        }
        long off = tz::utc_offset(tm->tm_isdst > 0 ? 1 : 0);
        *l = (size_t)snprintf_wrap(*s, sizeof *s, "%+.4ld",
                                   off / 3600 * 100 + off % 3600 / 60);
        return *s;
      }
      case 'Z':
        if (tm->tm_isdst < 0) {
          *l = 0;
          return "";
        }
        fmt = tz::get().tzname[tm->tm_isdst > 0 ? 1 : 0];
        goto string;
      case '%':
        *l = 1;
        return "%";
      default:
        return 0;
      }
    number:
      switch (pad ? pad : def_pad) {
      case '-':
        *l = (size_t)snprintf_wrap(*s, sizeof *s, "%lld", val);
        break;
      case '_':
        *l = (size_t)snprintf_wrap(*s, sizeof *s, "%*lld", width, val);
        break;
      case '0':
      default:
        *l = (size_t)snprintf_wrap(*s, sizeof *s, "%0*lld", width, val);
        break;
      }
      return *s;
    string:
      *l = strlen(fmt);
      return fmt;
    recu_strftime:
      *l = strftime_engine(*s, sizeof *s, fmt, tm);
      if (!*l)
        return 0;
      return *s;
    }

    namespace
    {
      size_t strftime_engine(char *s, size_t n, const char *f,
                             const struct tm *tm)
      {
        size_t l, k;
        char buf[100];
        const char *t;
        int pad, plus;
        unsigned long width;
        for (l = 0; l < n; f++) {
          if (!*f) {
            s[l] = 0;
            return l;
          }
          if (*f != '%') {
            s[l++] = *f;
            continue;
          }
          f++;
          pad = 0;
          if (*f == '-' || *f == '_' || *f == '0')
            pad = *f++;
          if ((plus = (*f == '+')))
            f++;
          /* musl: width = strtoul(f, &p, 10) — hand-rolled; p mirrors
           * musl's p (the post-width scan position, BEFORE the E/O
           * skip, which the '+'-flag decision below re-reads) */
          width = 0;
          int has_w = 0;
          if (is_digit(*f)) {
            has_w = 1;
            while (is_digit(*f)) {
              width = width * 10 + (unsigned long)(*f++ - '0');
              if (width > 0x10000000UL) /* absurd padding: saturate
                                         * (musl strtoul would too) */
                width = 0x10000000UL;
            }
          }
          const char *p = f;
          if (*p == 'C' || *p == 'F' || *p == 'G' || *p == 'Y') {
            if (!width && has_w)
              width = 1;
          } else {
            width = 0;
          }
          if (*f == 'E' || *f == 'O')
            f++;
          t = __strftime_fmt_1(&buf, &k, *f, tm, pad);
          if (!t)
            break;
          if (width) {
            /* Trim off any sign and leading zeros, then
             * count remaining digits to determine behavior
             * for the + flag. */
            if (*t == '+' || *t == '-')
              t++, k--;
            for (; *t == '0' && t[1] - '0' < 10U; t++, k--)
              ;
            if (width < k)
              width = k;
            size_t d;
            for (d = 0; t[d] - '0' < 10U; d++)
              ;
            if (tm->tm_year < -1900) {
              s[l++] = '-';
              width--;
            } else if (plus && d + (width - k) >= (*p == 'C' ? 3 : 5)) {
              s[l++] = '+';
              width--;
            }
            for (; width > k && l < n; width--)
              s[l++] = '0';
          }
          if (k > n - l)
            k = n - l;
          memcpy(s + l, t, k);
          l += k;
        }
        if (n) {
          if (l == n)
            l = n - 1;
          s[l] = 0;
        }
        return 0;
      }
    } // namespace

    size_t strftime(char *s, size_t n, const char *f, const struct tm *tm)
    {
      return strftime_engine(s, n, f, tm);
    }
  } // namespace musl
} // namespace mingw_thunk
