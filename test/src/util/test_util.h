//
//

#ifndef FS2_OPEN_TEST_UTIL_H
#define FS2_OPEN_TEST_UTIL_H

#include <gtest/gtest.h>

// This macro skips the following test if we are not in debug mode
// useful for things like parsing tests where there are no warnings in release mode
#ifdef NDEBUG
// This case needs an if condition to fix a MSVC warning which rightfully warns that the following code is unreachable
#define DEBUG_TEST() do { GTEST_SUCCEED(); if (1 == 1) { return; } } while (false)
#else
#define DEBUG_TEST() do {  } while (false)
#endif

#endif //FS2_OPEN_TEST_UTIL_H
