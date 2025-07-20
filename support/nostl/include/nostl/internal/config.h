#pragma once

#ifndef NS_NOSTL
#define NS_NOSTL nostl
#endif

#ifdef NOSTL_NOCRT
#ifndef _WIN32
#error "Building without CRT is only supported on Windows"
#endif
#define NS_NOSTL_CRT NS_NOCRT
#else
#define NS_NOSTL_CRT
#endif
