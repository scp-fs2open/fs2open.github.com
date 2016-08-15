
#include <gtest/gtest.h>

#include <globalincs/flagset.h>

FLAG_LIST(TestFlags)
{
	First = 0,
	Second,
	Another,
	AndAnother,

	NUM_VALUES
};

TEST(FlagsetTests, initializer_list) {
	{
		flagset<TestFlags> flags{};

		ASSERT_EQ(false, flags[TestFlags::First]);
		ASSERT_EQ(false, flags[TestFlags::Second]);
		ASSERT_EQ(false, flags[TestFlags::Another]);
		ASSERT_EQ(false, flags[TestFlags::AndAnother]);
	}
	{
		flagset<TestFlags> flags{ TestFlags::First};

		ASSERT_EQ(true, flags[TestFlags::First]);
		ASSERT_EQ(false, flags[TestFlags::Second]);
		ASSERT_EQ(false, flags[TestFlags::Another]);
		ASSERT_EQ(false, flags[TestFlags::AndAnother]);
	}
}

TEST(FlagsetTests, two_flags) {
	{
		flagset<TestFlags> flags{ TestFlags::First, TestFlags::Second };

		ASSERT_EQ(true, (flags[TestFlags::First, TestFlags::Second]));
		ASSERT_EQ(false, (flags[TestFlags::First, TestFlags::Another]));
		ASSERT_EQ(false, (flags[TestFlags::AndAnother, TestFlags::Another]));
	}
}

TEST(FlagsetTests, three_flags) {
	{
		flagset<TestFlags> flags{ TestFlags::First, TestFlags::Second, TestFlags::Another };

		ASSERT_EQ(true, (flags[TestFlags::First, TestFlags::Second, TestFlags::Another]));
		ASSERT_EQ(false, (flags[TestFlags::First, TestFlags::Second, TestFlags::AndAnother]));
	}
}

