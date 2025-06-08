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
 * @brief Macros to abstract compiler capabilities for the Clang toolchain
 *
 * @internal
 * This file should never be included directly; instead, one should
 * include toolchain.h which will pull in the file appropriate to
 * the detected toolchain.
 */
#pragma once

#if defined(__clang__)

#define SCP_FORMAT_STRING
#define SCP_FORMAT_STRING_ARGS(x,y)  __attribute__((format(printf, x, y)))

#define __UNUSED __attribute__((__unused__))
#define __ALIGNED(x)  __attribute__((__aligned__(x)))

#ifdef NO_RESTRICT_USE
#	define RESTRICT
#else
#	define RESTRICT  restrict
#endif

#define ASSUME(x)

#if defined(NDEBUG)
#	define Assertion(expr, msg, ...)  do { (void)sizeof(expr); } while (false)
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
	/*
	 * Clang does not seem to have a feature check for 'is_trivial'.
	 * Assume it will be covered by one of the following checks ...
	 * http://clang.llvm.org/docs/LanguageExtensions.html#feature_check
	 */
#	if __has_feature(cxx_static_assert) && __has_feature(cxx_auto_type)
#		define HAVE_CXX11
#	endif
#endif

#define SIZE_T_ARG    "%zu"
#define PTRDIFF_T_ARG "%zd"

#define UINT64_T_ARG  "%" PRIu64
#define  INT64_T_ARG  "%" PRId64

#define likely(x)    __builtin_expect((long) !!(x), 1L)
#define unlikely(x)  __builtin_expect((long) !!(x), 0L)

#define USED_VARIABLE __attribute__((used))

#if __has_cpp_attribute(fallthough)
#define FALLTHROUGH [[fallthrough]]
#elif __has_cpp_attribute(clang::fallthough)
#define FALLTHROUGH [[clang::fallthrough]]
#else
#define FALLTHROUGH
#endif

#ifndef CLANG_ANALYZER_NORETURN
#if __has_feature(attribute_analyzer_noreturn)
#define CLANG_ANALYZER_NORETURN __attribute__((analyzer_noreturn))
#else
#define CLANG_ANALYZER_NORETURN
#endif
#endif

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
_Pragma("clang diagnostic push") \
_Pragma("clang diagnostic ignored \"-Wattributes\"") \

/**
 * @brief Restored previous warning settings
 */
#define POP_SUPPRESS_WARNINGS \
_Pragma("clang diagnostic pop")

#endif
