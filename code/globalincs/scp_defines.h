
#ifndef SCP_DEFINES_H
#define SCP_DEFINES_H
#pragma once

#define SCP_TOKEN_CONCAT1(x, y) x ## y
#define SCP_TOKEN_CONCAT(x, y) SCP_TOKEN_CONCAT1(x, y)

// these are useful for embedding numbers in strings
// see https://stackoverflow.com/questions/5459868/concatenate-int-to-string-using-c-preprocessor
#define SCP_TOKEN_TO_STR1(x) #x
#define SCP_TOKEN_TO_STR(x) SCP_TOKEN_TO_STR1(x)

#define SCP_UNUSED(x) (void)(x)

#endif // SCP_DEFINES_H
