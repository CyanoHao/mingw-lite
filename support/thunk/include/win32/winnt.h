#pragma once

#include <minwindef.h>

typedef struct _NT_TIB
{
  struct _EXCEPTION_REGISTRATION_RECORD *ExceptionList;
  void *StackBase;
  void *StackLimit;
  void *SubSystemTib;
  union
  {
    void *FiberData;
    DWORD Version;
  };
  void *ArbitraryUserPointer;
  struct _NT_TIB *Self;
} NT_TIB;

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(__i386__)

  inline __attribute__((__always_inline__)) struct _TEB *NtCurrentTeb(void)
  {
    struct _TEB *teb;
    __asm__("movl %%fs:0x18, %0" : "=r"(teb));
    return teb;
  }

#elif defined(__x86_64__)

inline __attribute__((__always_inline__)) struct _TEB *NtCurrentTeb(void)
{
  struct _TEB *teb;
  __asm__("movq %%gs:0x30, %0" : "=r"(teb));
  return teb;
}

#elif defined(__aarch64__)

inline __attribute__((__always_inline__)) struct _TEB *NtCurrentTeb(void)
{
  register struct _TEB *teb __asm__("x18");
  return teb;
}

#else

#error Unsupported architecture

#endif

#ifdef __cplusplus
}
#endif
