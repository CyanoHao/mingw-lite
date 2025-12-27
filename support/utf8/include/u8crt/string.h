#pragma once

typedef __SIZE_TYPE__ size_t;

#ifdef __cplusplus
extern "C"
{
#endif

  int memcmp(const void *lhs, const void *rhs, size_t count);

#ifdef __cplusplus
}
#endif
