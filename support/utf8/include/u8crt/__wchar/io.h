#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

  int _wopen(const wchar_t *path, int flags, ... /* int mode */);

#ifdef __cplusplus
}
#endif
