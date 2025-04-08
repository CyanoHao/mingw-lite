#include <thunk/_common.h>
#include <thunk/os.h>

#include <errhandlingapi.h>
#include <windows.h>
#include <winternl.h>

#include <stdint.h>

namespace mingw_thunk
{
  // https://fanael.github.io/stockfish-on-windows-98.html

  struct win9x_critical_section
  {
    uint8_t type; // Always 4.
    struct win9x_critical_section_impl *impl;
    // Padding for NT compatibility, where the structure is completely
    // different.
    uint32_t reserved[4];
  };
  static_assert(sizeof(win9x_critical_section) == sizeof(CRITICAL_SECTION));

  struct win9x_critical_section_impl
  {
    uint8_t type;
    int recursion_count;
    uintptr_t owner_thread;
    uint32_t reserved;
    int lock_count; // Starts at 1 when not owned, decreasing.
    void *internal_pointers[3];
  };
  static_assert(sizeof(win9x_critical_section_impl) == 32);

  __DEFINE_THUNK(kernel32,
                 4,
                 BOOL,
                 WINAPI,
                 TryEnterCriticalSection,
                 _Inout_ LPCRITICAL_SECTION lpCriticalSection)
  {
    if (internal::is_nt())
      return get_TryEnterCriticalSection()(lpCriticalSection);

    // 95, 98: 0x50; Me: 0x80
    static const ptrdiff_t tdbx_offset =
        internal::os_version().dwMinorVersion > 10 ? 0x80 : 0x50;

    const auto actual =
        reinterpret_cast<win9x_critical_section *>(lpCriticalSection);
    if (actual->type != 4) [[unlikely]] {
      RaiseException(EXCEPTION_ACCESS_VIOLATION, 0, 0, 0);
      return FALSE;
    }

    TEB *teb = NtCurrentTeb();
    uintptr_t current_tdbx =
        *reinterpret_cast<uintptr_t *>(uintptr_t(teb) + tdbx_offset);

    win9x_critical_section_impl *cs = actual->impl;
    int actual_lock_count = 1;
    if (__atomic_compare_exchange_n(&cs->lock_count,
                                    &actual_lock_count,
                                    0,
                                    false,
                                    __ATOMIC_SEQ_CST,
                                    __ATOMIC_SEQ_CST)) {
      cs->owner_thread = current_tdbx;
      ++cs->recursion_count;
      return TRUE;
    } else if (cs->owner_thread == current_tdbx) {
      __atomic_fetch_sub(&cs->lock_count, 1, __ATOMIC_RELAXED);
      ++cs->recursion_count;
      return TRUE;
    } else {
      return FALSE;
    }
  }
} // namespace mingw_thunk
