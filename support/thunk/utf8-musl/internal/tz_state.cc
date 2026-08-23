/* C.UTF-8 timezone state (plan-2 M9 §4.3): the single truth source
 * for the UCRT tz globals (_tzset / __tzname / __daylight / __dstbias
 * / __timezone / _get_tzname) and the strftime %z/%Z/%s specifiers.
 *
 * Precedence (wine-anchored 2026-09-27):
 *  - TZ environment variable, POSIX subset "std offset [dst [offset]]"
 *    (alphabetic name >= 3 chars, [+|-]hh[:mm[:ss]]).  POSIX sign
 *    convention: the offset is ADDED to local to get UTC — EST5 ->
 *    UTC-5 -> _timezone = +18000; XXX-8 -> -28800 (both anchors
 *    confirmed).  Unparseable TZ -> UTC (wine anchor: TZ=bogus ->
 *    timezone 0).  DST without an explicit offset defaults to
 *    std-1h -> _dstbias = -3600; an explicit dst offset stores the
 *    DIFFERENCE from std.
 *  - no TZ (or empty) -> GetTimeZoneInformation: _timezone = (Bias +
 *    StandardBias) * 60 (positive west, seconds), _daylight =
 *    DaylightDate month != 0, _dstbias = DaylightBias * 60 when DST
 *    is observed, Std/Dlt names transcoded to UTF-8 (the registry
 *    English names — wine anchor: "China Standard Time" /
 *    "China Daylight Time").
 *
 * TZ is read via GetEnvironmentVariableA (kernel32): native putenv
 * keeps the Win32 environment in sync (the M9 probe validated the
 * putenv + re-read chain under wine), while the frozen musl
 * __environ snapshot would miss later mutations.
 *
 * Dynamic DST registry refinement is out of scope for the M9
 * surface: GetTimeZoneInformation already reports the effective
 * per-year bias on real Windows; only historical accuracy would
 * need the Dynamic DST key. */

#include <thunk/u8crt/musl.h>

#include <windows.h>

#include <string.h>

namespace mingw_thunk
{
  namespace musl
  {
    namespace tz
    {
      namespace
      {
        state g_state;
        bool g_inited;

        void wname_to_u8(const WCHAR *src, char *dst, size_t cap)
        {
          dst[0] = 0;
          if (src && src[0])
            WideCharToMultiByte(CP_UTF8, 0, src, -1, dst, (int)cap,
                                NULL, NULL);
        }

        void set_utc(state &st)
        {
          st.timezone = 0;
          st.dstbias = 0;
          st.daylight = 0;
          strcpy(st.std_name, "UTC");
          st.dlt_name[0] = 0;
        }

        /* [+|-]hh[:mm[:ss]] -> seconds (POSIX sign: positive = west) */
        bool parse_offset(const char *&p, long *secs)
        {
          long sign = 1;
          if (*p == '+' || *p == '-') {
            if (*p == '-')
              sign = -1;
            ++p;
          }
          if (!(*p >= '0' && *p <= '9'))
            return false;
          static const long mult[3] = {3600, 60, 1};
          long total = 0;
          for (int i = 0; i < 3; i++) {
            long x = 0;
            while (*p >= '0' && *p <= '9')
              x = x * 10 + (*p++ - '0');
            total += x * mult[i];
            if (*p != ':')
              break;
            ++p;
          }
          *secs = sign * total;
          return true;
        }

        void copy_name(char *dst, const char *src, size_t n)
        {
          if (n > sizeof g_state.std_name - 1)
            n = sizeof g_state.std_name - 1;
          memcpy(dst, src, n);
          dst[n] = 0;
        }

        bool parse_tz(const char *tz, state &st)
        {
          const char *p = tz;
          const char *std0 = p;
          while ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z'))
            ++p;
          size_t stdlen = (size_t)(p - std0);
          if (stdlen < 3)
            return false;
          long off;
          if (!parse_offset(p, &off))
            return false;
          st.timezone = off;
          st.daylight = 0;
          st.dstbias = 0;
          copy_name(st.std_name, std0, stdlen);
          st.dlt_name[0] = 0;
          const char *d0 = p;
          while ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z'))
            ++p;
          size_t dlen = (size_t)(p - d0);
          if (dlen >= 3) {
            st.daylight = 1;
            st.dstbias = -3600;
            copy_name(st.dlt_name, d0, dlen);
            if (*p == '+' || *p == '-' || (*p >= '0' && *p <= '9')) {
              long doff;
              if (parse_offset(p, &doff))
                st.dstbias = doff - off;
            }
          }
          return true;
        }

        void build(state &st)
        {
          st.tzname[0] = st.std_name;
          st.tzname[1] = st.dlt_name;

          char tzbuf[256];
          DWORD r = GetEnvironmentVariableA("TZ", tzbuf, sizeof tzbuf);
          if (r > 0 && r < sizeof tzbuf) {
            if (parse_tz(tzbuf, st))
              return;
            set_utc(st);
            return;
          }

          TIME_ZONE_INFORMATION tzi;
          if (GetTimeZoneInformation(&tzi) != TIME_ZONE_ID_INVALID) {
            st.timezone =
                ((long)tzi.Bias + (long)tzi.StandardBias) * 60;
            st.daylight = tzi.DaylightDate.wMonth != 0;
            st.dstbias =
                st.daylight ? (long)tzi.DaylightBias * 60 : 0;
            wname_to_u8(tzi.StandardName, st.std_name,
                        sizeof st.std_name);
            wname_to_u8(tzi.DaylightName, st.dlt_name,
                        sizeof st.dlt_name);
            return;
          }
          set_utc(st);
        }
      } // namespace

      state &get() noexcept
      {
        if (!g_inited) {
          build(g_state);
          g_inited = true;
        }
        return g_state;
      }

      void reinit() noexcept
      {
        build(g_state);
        g_inited = true;
      }

      long utc_offset(int isdst) noexcept
      {
        const state &st = get();
        return -(st.timezone + (isdst > 0 ? st.dstbias : 0));
      }
    } // namespace tz
  } // namespace musl
} // namespace mingw_thunk
