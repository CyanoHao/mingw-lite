#pragma once

typedef struct FILE FILE;

#ifdef __cplusplus
extern "C"
{
#endif

  int fclose(FILE *stream);
  int fopen(const char *__restrict path, const char *__restrict mode);

#ifdef __cplusplus
}
#endif

#include "__wchar/stdio.h"
