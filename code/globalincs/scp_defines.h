
#ifndef SCP_DEFINES_H
#define SCP_DEFINES_H
#pragma once

#define SCP_TOKEN_CONCAT1(x, y) x ## y
#define SCP_TOKEN_CONCAT(x, y) SCP_TOKEN_CONCAT1(x, y)

#define SCP_UNUSED(x) (void)(x)

#endif // SCP_DEFINES_H
