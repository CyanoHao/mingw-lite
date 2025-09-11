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
                 TryAcquireSRWLockShared,
                 _Inout_ PSRWLOCK SRWLock)
  {
    if (auto const pTryAcquireSRWLockShared =
            try_get_TryAcquireSRWLockShared()) {
      return pTryAcquireSRWLockShared(SRWLock);
    }

    // 尝试给全新的锁加锁
    auto OldSRWLock = __sync_val_compare_and_swap(
        (volatile size_t *)SRWLock, size_t(0), size_t(0x11));

    // 成功
    if (OldSRWLock == size_t(0)) {
      return TRUE;
    }

    for (;;) {
      if ((OldSRWLock & internal::YY_SRWLOCK_Locked) &&
          ((OldSRWLock & internal::YY_SRWLOCK_Waiting) ||
           internal::YY_SRWLOCK_GET_BLOCK(OldSRWLock) == nullptr)) {
        // 正在被锁定中
        return FALSE;
      } else {
        size_t NewSRWLock;

        if (OldSRWLock & internal::YY_SRWLOCK_Waiting)
          NewSRWLock = OldSRWLock | internal::YY_SRWLOCK_Locked;
        else
          NewSRWLock = (OldSRWLock + 0x10) | internal::YY_SRWLOCK_Locked;

        if (__sync_val_compare_and_swap((volatile size_t *)SRWLock,
                                        OldSRWLock,
                                        NewSRWLock) == OldSRWLock) {
          // 锁定完成
          return TRUE;
        }

        // RtlBackoff((unsigned int *)&v4);
        YieldProcessor();
        OldSRWLock = *(volatile size_t *)SRWLock;
      }
    }
  }
} // namespace mingw_thunk
