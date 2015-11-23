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
 * @brief Macros to abstract compiler capabilities for the Mingw toolchain
 *
 * @internal
 * This file should never be included directly; instead, one should
 * include toolchain.h which will pull in the file appropriate to
 * the detected toolchain.
 */

#define SCP_FORMAT_STRING
#define SCP_FORMAT_STRING_ARGS(x,y)  __attribute__((format(printf, x, y)))

#define __UNUSED __attribute__((__unused__))
#define __ALIGNED(x)  __attribute__((__aligned__(x)))

#ifdef NO_RESTRICT_USE
#   define RESTRICT
#else
#   define RESTRICT  restrict
#endif

#define ASSUME(x)

#if defined(NDEBUG)
#	define Assertion(expr, msg, ...)  do {} while (0)
#else
/*
 * NOTE: Assertion() can only use its proper functionality in compilers
 * that support variadic macros.
 */
#	define Assertion(expr, msg, ...)                                      \
		do {                                                              \
			if (!(expr)) {                                                \
				WinAssert(#expr, __FILE__, __LINE__, msg, ##__VA_ARGS__); \
			}                                                             \
		} while (0)
#endif

/* C++11 Standard Detection */
#if !defined(HAVE_CXX11)
	/* TODO */
#endif


#define SIZE_T_ARG    "%zu"
#define PTRDIFF_T_ARG "%zd"

#define NOEXCEPT  noexcept

#define likely(x)    __builtin_expect((long) !!(x), 1L)
#define unlikely(x)  __builtin_expect((long) !!(x), 0L)
