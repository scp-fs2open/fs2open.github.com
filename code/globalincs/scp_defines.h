
#ifndef SCP_DEFINES_H
#define SCP_DEFINES_H
#pragma once

#include "scp_compiler_detection.h"
#include "platformChecks.h"

#define SCP_TOKEN_CONCAT1(x, y) x ## y
#define SCP_TOKEN_CONCAT(x, y) SCP_TOKEN_CONCAT1(x, y)

#endif // SCP_DEFINES_H
