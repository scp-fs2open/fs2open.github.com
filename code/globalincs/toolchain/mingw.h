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

#if defined(__MINGW32__)

#include <stdio.h>

#define SCP_FORMAT_STRING
#define SCP_FORMAT_STRING_ARGS(x,y)  __attribute__((format(__MINGW_PRINTF_FORMAT, x, y)))

#define __UNUSED __attribute__((__unused__))
#define __ALIGNED(x)  __attribute__((__aligned__(x)))

#ifdef NO_RESTRICT_USE
#   define RESTRICT
#else
#   define RESTRICT  restrict
#endif

#define ASSUME(x)

#if defined(NDEBUG)
#	define Assertion(expr, msg, ...)  do {} while (false)
#else
/*
 * NOTE: Assertion() can only use its proper functionality in compilers
 * that support variadic macros.
 */
#	define Assertion(expr, msg, ...)                                      \
		do {                                                              \
			if (!(expr)) {                                                \
				os::dialogs::AssertMessage(#expr, __FILE__, __LINE__, msg, ##__VA_ARGS__); \
			}                                                             \
		} while (false)
#endif

/* C++11 Standard Detection */
#if !defined(HAVE_CXX11)
	/* TODO */
#endif


#define SIZE_T_ARG    "%zu"
#define PTRDIFF_T_ARG "%zd"

#define likely(x)    __builtin_expect((long) !!(x), 1L)
#define unlikely(x)  __builtin_expect((long) !!(x), 0L)

#define USED_VARIABLE __attribute__((used))

#if __GNUC__ >= 7
#define FALLTHROUGH __attribute__((fallthrough))
#else
#define FALLTHROUGH
#endif

#define CLANG_ANALYZER_NORETURN

#ifndef NDEBUG
#define UNREACHABLE(msg, ...)                                                                                          \
	do {                                                                                                               \
		os::dialogs::Error(__FILE__, __LINE__, msg, ##__VA_ARGS__);                                                    \
	} while (false)
#else
#define UNREACHABLE(msg, ...) __builtin_unreachable()
#endif

/**
 * @brief Suppresses all warnings and allows to pop back to normal afterwards
 */
#define PUSH_SUPPRESS_WARNINGS \
_Pragma("GCC diagnostic push") \
_Pragma("GCC diagnostic ignored \"-Wattributes\"") \

/**
 * @brief Restored previous warning settings
 */
#define POP_SUPPRESS_WARNINGS \
_Pragma("GCC diagnostic pop")

#endif
