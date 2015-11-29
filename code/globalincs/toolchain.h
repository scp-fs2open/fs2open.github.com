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
 * @brief Macros to abstract compiler capabilities for various toolchains
 */

#ifndef _TOOLCHAIN_H
#define _TOOLCHAIN_H

#if defined(DOXYGEN)
#	include "globalincs/toolchain/doxygen.h"
#elif defined(__MINGW32__)
#	include "globalincs/toolchain/mingw.h"
#elif defined(__clang__)
#	include "globalincs/toolchain/clang.h"
#elif defined(__GNUC__)
#	include "globalincs/toolchain/gcc.h"
#elif defined(_MSC_VER)
#	include "globalincs/toolchain/msvc.h"
#else
#	error "Unknown toolchain detected!\n"           \
		"Currently supported toolchains include:\n" \
		"\tMingW, Clang, GCC, MSVC\n"               \
		"Update toolchain.h to add support for additional toolchains.\n"
#endif

#endif /* _TOOLCHAIN_H */
