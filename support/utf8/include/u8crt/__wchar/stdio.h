#pragma once

typedef struct FILE FILE;

#ifdef __cplusplus
extern "C"
{
#endif

  FILE *_wfopen(const wchar_t *__restrict path, const wchar_t *__restrict mode);

#ifdef __cplusplus
}
#endif
