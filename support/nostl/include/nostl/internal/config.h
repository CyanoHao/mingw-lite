#pragma once

#include <windows.h>

#ifndef NS_NOSTL
  #define NS_NOSTL nostl
#endif

#ifdef NOSTL_NOCRT
  #ifndef _WIN32
    #error "Building without CRT is only supported on Windows"
  #endif
  #define NS_NOSTL_CRT NS_NOCRT
#else
  #define NS_NOSTL_CRT
#endif

#ifndef NOSTL_RAISE_BAD_ALLOC
  #ifdef _WIN32
    #define NOSTL_RAISE_BAD_ALLOC()                                            \
      RaiseException(STATUS_NO_MEMORY, 0, 0, nullptr)
  #else
    #define NOSTL_RAISE_BAD_ALLOC() raise(SIGSEGV)
  #endif
#endif
