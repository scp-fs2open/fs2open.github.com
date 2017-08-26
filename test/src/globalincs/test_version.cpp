//
//

#include <gtest/gtest.h>

#include "globalincs/version.h"

using namespace gameversion;

TEST(VersionTest, equal_version) {
	ASSERT_TRUE(version(1, 2, 3, 4) == version(1, 2, 3, 4));
	ASSERT_TRUE(version(1, 2, 3, 0) == version(1, 2, 3, 20));

	ASSERT_FALSE(version(1, 2, 3, 4) != version(1, 2, 3, 4));
}

TEST(VersionTest, inequal_version) {
	ASSERT_TRUE(version(1, 2, 3, 4) != version(4, 3, 2, 1));

	ASSERT_FALSE(version(1, 2, 3, 4) == version(4, 3, 2, 1));
	ASSERT_FALSE(version(1, 2, 3, 0) == version(4, 3, 2, 0));
}

TEST(VersionTest, lesser_version) {
	ASSERT_TRUE(version(2, 0, 0, 0) < version(3, 0, 0, 0));
	ASSERT_TRUE(version(2, 1, 0, 0) < version(2, 2, 0, 0));
	ASSERT_TRUE(version(2, 1, 0, 0) < version(2, 1, 1, 0));
	ASSERT_TRUE(version(2, 1, 0, 1) < version(2, 1, 0, 2));

	ASSERT_FALSE(version(2, 1, 0, 0) < version(2, 1, 0, 1)); // Special case, no revision
	ASSERT_FALSE(version(2, 0, 0, 0) < version(2, 0, 0, 0));
	ASSERT_FALSE(version(2, 0, 0, 0) < version(1, 0, 0, 0));
}
