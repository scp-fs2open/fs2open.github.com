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
 * @brief Macros to abstract compiler capabilities (for doxygen use only)
 */

/**
 * @brief Identifies a printf-style format string
 */
#define SCP_FORMAT_STRING

/**
 * @brief Specify which arguments are involved in printf-style string formatting
 *
 * @details Expands to a compiler specific attribute which specify where the
 *          format arguments are located. Parameters are 1-based which also
 *          includes the 'this' parameter at position 1 for class methods.
 *
 * @param formatArg Location of format string argument in the argument list
 * @param varArgs Location where the variable arguments begin
 */
#define SCP_FORMAT_STRING_ARGS(formatArg,varArgs)

/**
 * @brief Format specifier for a @c size_t argument
 *
 * Due to different runtimes using different format specifier for these types
 * it's necessary to hide these changes behind a macro. Use this in place of %zu
 */
#define SIZE_T_ARG    "%zu"

/**
 * @brief Format specifier for a @c ptrdiff_t argument
 *
 * Due to different runtimes using different format specifier for these types
 * it's necessary to hide these changes behind a macro. Use this in place of %zd
 */
#define PTRDIFF_T_ARG "%zd"
