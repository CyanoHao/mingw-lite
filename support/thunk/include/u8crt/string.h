#pragma once

typedef __SIZE_TYPE__ size_t;

#ifdef __cplusplus
extern "C"
{
#endif

  int memcmp(const void *s1, const void *s2, size_t n);

#ifdef __cplusplus
}
#endif
