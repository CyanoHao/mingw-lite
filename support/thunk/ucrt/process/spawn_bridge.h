#pragma once

// Shared core for the sixteen narrow _exec*/_spawn* faces (plan §M12).
// Every entry ends up in the same two steps:
//
//   1. UTF-8 -> UTF-16 for the file name and the environment vector
//      (our narrow strings are UTF-8; the plain wide natives are the
//      delegation target, exactly the _wfopen / _wsystem precedent).
//   2. one call into _wspawnve / _wspawnvpe / _wexecve / _wexecvpe, which
//      already own the mode dispatch, the inherited-handle block, the
//      extension probing and the %PATH% search.
//
// The -l/-le/-lp/-lpe varargs faces additionally reproduce
// exec/cenvarg.cpp: common_capture_argv walks the va_list into a
// 64-entry caller array first and only falls back to the heap when that
// overflows.  The _exec family is the same core as _spawn with mode
// _P_OVERLAY, which is how the reference header defines it.
//
// Internal header; only the process shell .cc files and their tests
// include it.

#include <thunk/_common.h>
#include <thunk/string.h>

#include <errno.h>
#include <process.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

namespace mingw_thunk
{
  namespace i
  {
    namespace proc
    {
      using w_argv = wchar_t const *const *;
      using w_envp = wchar_t const *const *;

      /* A NULL-terminated wchar_t* vector.  Every entry owns what it
       * built, so a failure half way through unwinds with the
       * destructor. */
      class w_vector
      {
      public:
        w_vector() noexcept = default;

        ~w_vector() noexcept { clear(); }

        w_vector(const w_vector &) = delete;
        w_vector &operator=(const w_vector &) = delete;

        /* Appends one UTF-8 string; a null pointer is kept as a null
         * entry (both the varargs terminator and the envp sentinel are
         * meaningful nulls).  An empty string is a legal one-unit
         * string, not a null entry. */
        bool push(const char *s) noexcept
        {
          if (!reserve())
            return false;

          wchar_t **slot = &slots_[count_];
          if (s == nullptr) {
            *slot = nullptr;
            ++count_;
            return true;
          }

          size_t len = c::strlen(s);
          if (len > size_t(INT_MAX))
            return fail(ERROR_INVALID_DATA);

          int units = d::w_str::size_from_u(s, int(len));
          if (units < 0)
            return fail(ERROR_INVALID_DATA);

          wchar_t *dst = static_cast<wchar_t *>(HeapAlloc(
              GetProcessHeap(), 0, (size_t(units) + 1) * sizeof(wchar_t)));
          if (dst == nullptr)
            return false;

          if (units > 0) {
            int got = d::w_str::fixed_buffer_from_u(dst, units, s, int(len));
            if (got != units) {
              HeapFree(GetProcessHeap(), 0, dst);
              return fail(ERROR_INVALID_DATA);
            }
          }
          dst[units] = L'\0';

          *slot = dst;
          ++count_;
          return true;
        }

        /* Closes the vector with its NULL sentinel.  The wide side
         * reads the entries until that sentinel, so the length need
         * not be carried across. */
        bool terminate() noexcept
        {
          if (!reserve())
            return false;
          slots_[count_] = nullptr;
          return true;
        }

        w_argv c() const noexcept { return slots_; }

      private:
        static bool fail(DWORD err) noexcept
        {
          SetLastError(err);
          return false;
        }

        bool reserve() noexcept
        {
          if (count_ + 1 < cap_)
            return true;

          size_t ncap = cap_ ? cap_ * 2 : 8;
          wchar_t **grown = static_cast<wchar_t **>(
              HeapAlloc(GetProcessHeap(), 0, ncap * sizeof(wchar_t *)));
          if (grown == nullptr)
            return false;

          for (size_t i = 0; i < count_; ++i)
            grown[i] = slots_[i];
          if (slots_ != nullptr)
            HeapFree(GetProcessHeap(), 0, slots_);

          slots_ = grown;
          cap_ = ncap;
          return true;
        }

        void clear() noexcept
        {
          if (slots_ != nullptr) {
            for (size_t i = 0; i < count_; ++i) {
              if (slots_[i] != nullptr)
                HeapFree(GetProcessHeap(), 0, slots_[i]);
            }
            HeapFree(GetProcessHeap(), 0, slots_);
          }
          slots_ = nullptr;
          count_ = 0;
          cap_ = 0;
        }

        wchar_t **slots_ = nullptr;
        size_t count_ = 0;
        size_t cap_ = 0;
      };

      /* Converts a main()-style narrow vector (argv or envp) into the
       * wide one.  A null vector stays null — that is what tells the
       * native to inherit the calling process's environment. */
      inline bool convert_vector(const char *const *src, w_vector &out) noexcept
      {
        if (src == nullptr)
          return true;
        for (const char *const *it = src; *it != nullptr; ++it) {
          if (!out.push(*it))
            return false;
        }
        return out.terminate();
      }

      /* exec/cenvarg.cpp: common_capture_argv.  Copies the varargs
       * pointers (never the strings) into the caller's 64-entry array,
       * doubling onto the heap when that overflows, then converts the
       * collected pointers.  Fails only on allocation failure or a
       * UTF-8 string that will not convert. */
      inline bool
      capture_argv(va_list *ap, const char *first, w_vector &out) noexcept
      {
        const char *stack_slots[64];
        const char **slots = stack_slots;
        size_t cap = sizeof stack_slots / sizeof stack_slots[0];
        const char **heap = nullptr;
        bool ok = true;

        size_t i = 0;
        const char *next = first;
        for (;;) {
          if (i >= cap) {
            /* the reference doubles until the slot count would pass
             * SIZE_MAX/2 and bails out with ENOMEM */
            if (cap > SIZE_MAX / 2 / sizeof(void *)) {
              _set_errno(ENOMEM);
              ok = false;
              break;
            }

            const char **grown = static_cast<const char **>(HeapAlloc(
                GetProcessHeap(), 0, cap * 2 * sizeof(void *)));
            if (grown == nullptr) {
              _set_errno(ENOMEM);
              ok = false;
              break;
            }

            for (size_t k = 0; k < i; ++k)
              grown[k] = slots[k];
            if (heap != nullptr)
              HeapFree(GetProcessHeap(), 0, heap);
            heap = grown;
            slots = grown;
            cap *= 2;
          }

          slots[i++] = next;
          if (next == nullptr)
            break;
          next = va_arg(*ap, const char *);
        }

        for (size_t k = 0; ok && k < i; ++k)
          ok = out.push(slots[k]);

        if (heap != nullptr)
          HeapFree(GetProcessHeap(), 0, heap);
        return ok;
      }

      /* The -l/-le half of exec/spawnl.cpp: the varargs shape with a
       * null terminator, and the 'e' variants' trailing envp vector. */
      inline bool capture_argl(bool pass_environment,
                               va_list *ap,
                               const char *first,
                               w_vector &out,
                               const char *const *&envp) noexcept
      {
        if (!capture_argv(ap, first, out))
          return false;
        if (!out.terminate())
          return false;

        envp = pass_environment ? va_arg(*ap, const char *const *) : nullptr;
        return true;
      }

      /* The one call site: transcode the file name and the environment
       * vector, then hand the whole call to the wide native.  `search`
       * selects the _w*vp pair (the %PATH% search lives natively in
       * exec/spawnvp.cpp); mode is _P_OVERLAY for the _exec* names.
       *
       * A null envp is passed through unchanged, so an 'e' entry with
       * a null envp is exactly the non-'e' entry — the identity the
       * reference header documents. */
      template <intptr_t (*Plain)(int, wchar_t const *, w_argv, w_envp),
                intptr_t (*Search)(int, wchar_t const *, w_argv, w_envp)>
      inline intptr_t spawn_w(int mode,
                              bool search,
                              const char *file,
                              w_argv argv,
                              const char *const *envp) noexcept
      {
        if (file == nullptr) {
          /* the reference reaches the invalid parameter handler here;
           * the graceful shape is the M11 precedent */
          _set_errno(EINVAL);
          return -1;
        }

        d::w_str w_file;
        if (!w_file.from_u(file)) {
          _set_errno(ENOMEM);
          return -1;
        }

        w_vector w_env;
        if (!convert_vector(envp, w_env)) {
          _set_errno(ENOMEM);
          return -1;
        }

        return search ? Search(mode, w_file.c_str(), argv, w_env.c())
                      : Plain(mode, w_file.c_str(), argv, w_env.c());
      }

      /* The _exec* half: no mode, the execv pair. */
      template <intptr_t (*Plain)(wchar_t const *, w_argv, w_envp),
                intptr_t (*Search)(wchar_t const *, w_argv, w_envp)>
      inline intptr_t exec_w(bool search,
                             const char *file,
                             w_argv argv,
                             const char *const *envp) noexcept
      {
        if (file == nullptr) {
          _set_errno(EINVAL);
          return -1;
        }

        d::w_str w_file;
        if (!w_file.from_u(file)) {
          _set_errno(ENOMEM);
          return -1;
        }

        w_vector w_env;
        if (!convert_vector(envp, w_env)) {
          _set_errno(ENOMEM);
          return -1;
        }

        return search ? Search(w_file.c_str(), argv, w_env.c())
                      : Plain(w_file.c_str(), argv, w_env.c());
      }

      /* The -v/-ve/-vp/-vpe half: the caller hands us a main()-style
       * narrow vector that is converted exactly like a captured one. */
      template <intptr_t (*Plain)(int, wchar_t const *, w_argv, w_envp),
                intptr_t (*Search)(int, wchar_t const *, w_argv, w_envp)>
      inline intptr_t spawn(int mode,
                            bool search,
                            const char *file,
                            const char *const *argv,
                            const char *const *envp) noexcept
      {
        w_vector w_argv;
        if (!convert_vector(argv, w_argv) || !w_argv.terminate()) {
          _set_errno(ENOMEM);
          return -1;
        }
        return spawn_w<Plain, Search>(mode, search, file, w_argv.c(), envp);
      }

      template <intptr_t (*Plain)(wchar_t const *, w_argv, w_envp),
                intptr_t (*Search)(wchar_t const *, w_argv, w_envp)>
      inline intptr_t exec(bool search,
                           const char *file,
                           const char *const *argv,
                           const char *const *envp) noexcept
      {
        w_vector w_argv;
        if (!convert_vector(argv, w_argv) || !w_argv.terminate()) {
          _set_errno(ENOMEM);
          return -1;
        }
        return exec_w<Plain, Search>(search, file, w_argv.c(), envp);
      }
    } // namespace proc
  } // namespace i
} // namespace mingw_thunk
