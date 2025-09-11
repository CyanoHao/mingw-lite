#include <thunk/_common.h>
#include <thunk/yy/api-ms-win-core-synch.h>

#include <synchapi.h>

namespace mingw_thunk
{
  // YY-Thunks 1.1.7
  // Windows 7 [desktop apps | UWP apps]
  // Windows Server 2008 R2 [desktop apps | UWP apps]
  __DEFINE_THUNK(kernel32,
                 4,
                 BOOLEAN,
                 WINAPI,
                 TryAcquireSRWLockExclusive,
                 _Inout_ PSRWLOCK SRWLock)
  {
    if (auto const pTryAcquireSRWLockExclusive =
            try_get_TryAcquireSRWLockExclusive()) {
      return pTryAcquireSRWLockExclusive(SRWLock);
    }

#if defined(_WIN64)
    return InterlockedBitTestAndSet64((volatile LONG_PTR *)SRWLock,
                                      internal::YY_SRWLOCK_Locked_BIT) == 0;
#else
    return InterlockedBitTestAndSet((volatile LONG_PTR *)SRWLock,
                                    internal::YY_SRWLOCK_Locked_BIT) == 0;
#endif
  }

} // namespace mingw_thunk
