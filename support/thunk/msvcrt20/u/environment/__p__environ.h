#pragma once

#include <stddef.h>

namespace mingw_thunk
{
  namespace i
  {
    extern char **u8_dup_envp(char **envp, size_t size);

    extern char **u8_envp;

    extern char **u8_environ;
    extern size_t u8_environ_size;
    extern bool u8_environ_lock;
  } // namespace i
} // namespace mingw_thunk
