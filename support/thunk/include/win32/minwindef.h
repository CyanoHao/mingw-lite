#pragma once

// stddef.h and stdint.h type aliases
typedef __INT8_TYPE__ int8_t;
typedef __INT16_TYPE__ int16_t;
typedef __INT32_TYPE__ int32_t;
typedef __INT64_TYPE__ int64_t;

typedef __UINT8_TYPE__ uint8_t;
typedef __UINT16_TYPE__ uint16_t;
typedef __UINT32_TYPE__ uint32_t;
typedef __UINT64_TYPE__ uint64_t;

typedef __SIZE_TYPE__ size_t;
typedef __INTPTR_TYPE__ intptr_t;
typedef __UINTPTR_TYPE__ uintptr_t;
typedef __PTRDIFF_TYPE__ ptrdiff_t;
typedef __WINT_TYPE__ wint_t;

#ifndef __cplusplus
typedef __WCHAR_TYPE__ wchar_t;
#endif

// Win32 fixed length types
typedef __UINT8_TYPE__ BYTE;
typedef __UINT32_TYPE__ DWORD;
typedef __UINT32_TYPE__ DWORD32;
typedef __UINT64_TYPE__ DWORD64;
typedef __UINT64_TYPE__ DWORDLONG;
typedef __UINTPTR_TYPE__ DWORD_PTR;

// Win32 type aliases
typedef int BOOL;
typedef void *HANDLE;
