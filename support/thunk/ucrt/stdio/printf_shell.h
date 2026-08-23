#pragma once

// Shared helpers for the __stdio_common_* shell layer (plan §M2):
// the per-call engine TLS state.  (%n is musl semantics here: the
// engine implements the conversion unconditionally and no pre-scan
// gate exists — the count-output switch is pinned on.)  Internal
// header; only the stdio shell .cc files include it.

#include <thunk/u8crt/musl.h>

#include <stdint.h>

namespace mingw_thunk
{
  namespace i
  {
    namespace shell
    {
      /* Raises the engine's minimum exponent digits to 3 for the
       * duration of one engine call when the caller requested legacy
       * three-digit exponents (options bit 0x10); restores the engine
       * default (2) on the way out. */
      struct exp_guard
      {
        bool armed;

        explicit exp_guard(uint64_t options) noexcept
            : armed((options & 0x10) != 0)
        {
          if (armed)
            musl::exp_digits_min = 3;
        }

        ~exp_guard()
        {
          if (armed)
            musl::exp_digits_min = 2;
        }

        exp_guard(const exp_guard &) = delete;
        exp_guard &operator=(const exp_guard &) = delete;
      };
    } // namespace shell
  } // namespace i
} // namespace mingw_thunk
