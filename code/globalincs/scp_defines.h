
#ifndef SCP_DEFINES_H
#define SCP_DEFINES_H
#pragma once

#include "scp_compiler_detection.h"
#include "platformChecks.h"

#define SCP_TOKEN_CONCAT1(x, y) x ## y
#define SCP_TOKEN_CONCAT(x, y) SCP_TOKEN_CONCAT1(x, y)

/**
 * Define for a function that should be constexpr but if the compiler doesn't support it then it uses inline
 */

#if SCP_COMPILER_CXX_CONSTEXPR
#define SCP_CONSTEXPR_FUNC constexpr
#else
#define SCP_CONSTEXPR_FUNC inline
#endif

#endif // SCP_DEFINES_H
