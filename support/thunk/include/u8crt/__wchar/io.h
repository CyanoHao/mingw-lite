#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

  int _wopen(const wchar_t *path, int flags, ... /* mode_t mode */);

#ifdef __cplusplus
}
#endif
