#pragma once

#define ENOMEM 12
#define EINVAL 22

#define errno (*__errno_location())

#ifdef __cplusplus
extern "C"
{
#endif

  int *__errno_location();

#ifdef __cplusplus
}
#endif
