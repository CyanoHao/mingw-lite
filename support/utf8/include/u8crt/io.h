#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

  int _open(const char * path, int flags, ... /* int mode */);

#ifdef __cplusplus
}
#endif

#include "__wchar/io.h"
