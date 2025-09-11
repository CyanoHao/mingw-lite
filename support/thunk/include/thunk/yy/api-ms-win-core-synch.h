#pragma once

#include <minwindef.h>

#include <stddef.h>
#include <stdint.h>

// YY-Thunks 1.1.7
namespace mingw_thunk::internal
{
  // 读写锁参考了https://blog.csdn.net/yichigo/article/details/36898561
  constexpr int YY_SRWLOCK_Locked_BIT = 0;
  constexpr int YY_SRWLOCK_Waiting_BIT = 1;
  constexpr int YY_SRWLOCK_Waking_BIT = 2;
  constexpr int YY_SRWLOCK_MultipleShared_BIT = 3;
  // 已经有人获得这个锁
  constexpr unsigned long YY_SRWLOCK_Locked = 0x00000001ul;
  // 有人正在等待锁
  constexpr unsigned long YY_SRWLOCK_Waiting = 0x00000002ul;
  // 有人正在唤醒锁
  constexpr unsigned long YY_SRWLOCK_Waking = 0x00000004ul;
  //
  constexpr unsigned long YY_SRWLOCK_MultipleShared = 0x00000008ul;

  constexpr size_t YY_SRWLOCK_MASK = 0xF;
  constexpr int YY_SRWLOCK_BITS = 4;
  // SRWLock自旋次数
  constexpr int SRWLockSpinCount = 1024;

  struct _YY_RTL_SRWLOCK
  {
    union
    {
      struct
      {
        ULONG_PTR Locked : 1;
        ULONG_PTR Waiting : 1;
        ULONG_PTR Waking : 1;
        ULONG_PTR MultipleShared : 1;
#ifdef _WIN64
        ULONG_PTR Shared : 60;
#else
        ULONG_PTR Shared : 28;
#endif
      };
      ULONG_PTR Value;
      VOID *Ptr;
    };
  };

  typedef struct alignas(16) _YY_SRWLOCK_WAIT_BLOCK
  {
    _YY_SRWLOCK_WAIT_BLOCK *back;
    _YY_SRWLOCK_WAIT_BLOCK *notify;
    _YY_SRWLOCK_WAIT_BLOCK *next;
    volatile size_t shareCount;
    volatile size_t flag;

    // 以下成员YY-Thunks特有

    // 故意填充的成员，便于于YY_CV_WAIT_BLOCK ABI兼容
    void *pReserved;
    // 唤醒此任务的线程Id
    volatile uint32_t uWakeupThreadId;
  } YY_SRWLOCK_WAIT_BLOCK;

  inline YY_SRWLOCK_WAIT_BLOCK *YY_SRWLOCK_GET_BLOCK(size_t SRWLock)
  {
    return (YY_SRWLOCK_WAIT_BLOCK *)(SRWLock & ~YY_SRWLOCK_MASK);
  }

} // namespace mingw_thunk::internal
