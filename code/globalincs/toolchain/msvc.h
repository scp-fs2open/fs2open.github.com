/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell
 * or otherwise commercially exploit the source or things you created based on
 * the source.
 */

/**
 * @file
 *
 * @brief Macros to abstract compiler capabilities for the MSVC toolchain
 *
 * @internal
 * This file should never be included directly; instead, one should
 * include toolchain.h which will pull in the file appropriate to
 * the detected toolchain.
 */

#include <sal.h>

#define SCP_FORMAT_STRING            _Printf_format_string_
#define SCP_FORMAT_STRING_ARGS(x,y)

#define __attribute__(x)
#define __UNUSED
#define __ALIGNED(x)  __declspec(align(x))

#ifdef NO_RESTRICT_USE
#   define RESTRICT
#elif _MSC_VER >= 1400
#   define RESTRICT  __restrict
#else
#   define RESTRICT
#endif

#define ASSUME(x)

#if defined(NDEBUG)
#	if _MSC_VER >= 1400  /* MSVC 2005 or newer */
#		define Assertion(expr, msg, ...)  do { ASSUME(expr); } while (0)
#	else
#		define Assertion(expr, msg)  do {} while (0)
#	endif
#else
	/*
	 * NOTE: Assertion() can only use its proper functionality in compilers
	 * that support variadic macros.
	 */
#	if _MSC_VER >= 1400  /* MSVC 2005 or newer */
#		define Assertion(expr, msg, ...)                                    \
			do {                                                            \
				if (!(expr)) {                                              \
					WinAssert(#expr, __FILE__, __LINE__, msg, __VA_ARGS__); \
				}                                                           \
			} while (0)
#	else                 /* Older MSVC compilers */
#		define Assertion(expr, msg)                        \
			do {                                           \
				if (!(expr)) {                             \
					WinAssert(#expr, __FILE__, __LINE__);  \
			} while (0)
#	endif
#endif

/* C++11 Standard Detection */
#if !defined(HAVE_CXX11)
	/* Use the Visual Studio version to detect C++11 support */
#	if _MSC_VER >= 1600
#		define HAVE_CXX11
#	endif
#endif

#define PTRDIFF_T_ARG "%Iu"
#define SIZE_T_ARG    "%Id"

/* The 'noexcept' keyword is not defined in versions before VS 2015. */
#if _MSC_VER < 1900
#	define NOEXCEPT
#else
#	define NOEXCEPT  noexcept
#endif

#define likely(x)
#define unlikely(x)
