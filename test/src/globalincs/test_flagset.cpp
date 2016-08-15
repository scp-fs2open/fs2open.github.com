
#include <gtest/gtest.h>

#include <globalincs/flagset.h>

FLAG_LIST(TestFlags)
{
	First = 0,
	Second,
	Another,
	AndAnother,
	One,
	Two,

	NUM_VALUES
};

TEST(FlagsetTests, initializer_list) {
	{
		flagset<TestFlags> flags{};

		ASSERT_FALSE(flags[TestFlags::First]);
		ASSERT_FALSE(flags[TestFlags::Second]);
		ASSERT_FALSE(flags[TestFlags::Another]);
		ASSERT_FALSE(flags[TestFlags::AndAnother]);
	}
	{
		flagset<TestFlags> flags{ TestFlags::First};

		ASSERT_TRUE(flags[TestFlags::First]);
		ASSERT_FALSE(flags[TestFlags::Second]);
		ASSERT_FALSE(flags[TestFlags::Another]);
		ASSERT_FALSE(flags[TestFlags::AndAnother]);
	}
}

TEST(FlagsetTests, two_flags) {
	{
		flagset<TestFlags> flags{ TestFlags::First, TestFlags::Second };

		ASSERT_TRUE((flags[TestFlags::First, TestFlags::Second]));
		ASSERT_TRUE((flags[TestFlags::First, TestFlags::Another]));
		ASSERT_FALSE((flags[TestFlags::AndAnother, TestFlags::Another]));
	}
}

TEST(FlagsetTests, three_flags) {
	{
		flagset<TestFlags> flags{ TestFlags::First, TestFlags::Second, TestFlags::Another };

		ASSERT_TRUE((flags[TestFlags::First, TestFlags::Second, TestFlags::Another]));
		ASSERT_FALSE((flags[TestFlags::One, TestFlags::Two, TestFlags::AndAnother]));
	}
}

