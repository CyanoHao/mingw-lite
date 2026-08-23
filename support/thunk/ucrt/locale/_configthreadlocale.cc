#include <thunk/_common.h>

namespace mingw_thunk
{
  // D11 state machine, reverse-engineered from wine across 27 probe
  // observations (safe group G + four discriminating sequences):
  //
  //   P = per-thread-configured flag, initially 0 (process-wide state)
  //   ctl(1) -> ret = enc(P), P = 1
  //   ctl(2) -> ret = enc(P), P = 0
  //   ctl(0) -> ret = enc(P), P unchanged
  //   other  -> -1 (errno untouched)
  //   enc(P) = P ? 1 : 2
  //
  // Observed replays: 0,1,0,2,0,-1,0 -> 2,2,1,1,2,-1,2;
  // 1,2,0,0 -> 2,1,2,2; 2,2,1,0 -> 2,2,2,1; 1,0,0 -> 2,1,1;
  // 1,42,0,-3,0 -> 2,-1,1,-1,1.
  //
  // The overlay never actually enables per-thread locale state (the
  // single locale is C.UTF-8 for the whole process); this is a protocol
  // clone so third-party state probes observe native shapes.
  __DEFINE_THUNK(
      api_ms_win_crt_locale_l1_1_0, 0, int, __cdecl, _configthreadlocale,
      int option)
  {
    static int per_thread_configured = 0;

    switch (option) {
    case 0:
      break;
    case 1:
    case 2: {
      int previous = per_thread_configured;
      per_thread_configured = option == 1 ? 1 : 0;
      return previous ? 1 : 2;
    }
    default:
      return -1;
    }

    return per_thread_configured ? 1 : 2;
  }
} // namespace mingw_thunk
