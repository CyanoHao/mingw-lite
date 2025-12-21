#pragma once

typedef __SIZE_TYPE__ size_t;

#ifdef __cplusplus
extern "C"
{
#endif

  void *calloc(size_t nmemb, size_t size);
  void free(void *ptr);
  void *malloc(size_t size);
  void *realloc(void *ptr, size_t size);

#ifdef __cplusplus
}
#endif
