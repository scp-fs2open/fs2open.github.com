
#include <gtest/gtest.h>

#include <parse/parselo.h>

#include "util/FSTestFixture.h"

class SplitStrTest : public test::FSTestFixture
{
	public:
		SplitStrTest() : test::FSTestFixture(INIT_CFILE | INIT_GRAPHICS | INIT_FONTS) {}
};

TEST_F(SplitStrTest, split_str_once_empty)
{
	{
		auto [len, pos, forced] = split_str_once(nullptr, 100);
		ASSERT_EQ(len, 0);
		ASSERT_EQ(pos, 0);
		ASSERT_EQ(forced, false);
	}

	{
		auto [len, pos, forced] = split_str_once("", 100);
		ASSERT_EQ(len, 0);
		ASSERT_EQ(pos, 0);
		ASSERT_EQ(forced, false);
	}
}

// per GitHub #5473, zero or negative width should not cause a split
TEST_F(SplitStrTest, split_str_once_zero)
{
	{
		auto [len, pos, forced] = split_str_once("abcde", 0);
		ASSERT_EQ(len, 5);
		ASSERT_EQ(pos, 0);
		ASSERT_EQ(forced, false);
	}

	{
		auto [len, pos, forced] = split_str_once("abc defg", 0);
		ASSERT_EQ(len, 8);
		ASSERT_EQ(pos, 0);
		ASSERT_EQ(forced, false);
	}

	{
		auto [len, pos, forced] = split_str_once("abc\tdefg", 0);
		ASSERT_EQ(len, 8);
		ASSERT_EQ(pos, 0);
		ASSERT_EQ(forced, false);
	}

	{
		auto [len, pos, forced] = split_str_once("abc\ndefg", 0);
		ASSERT_EQ(len, 8);
		ASSERT_EQ(pos, 0);
		ASSERT_EQ(forced, false);
	}

	{
		auto [len, pos, forced] = split_str_once("abc defg", -1);
		ASSERT_EQ(len, 8);
		ASSERT_EQ(pos, 0);
		ASSERT_EQ(forced, false);
	}
}

TEST_F(SplitStrTest, split_str_once_typical)
{
	// note - in the default retail font, most characters are 8 pixels wide

	// no split
	{
		auto [len, pos, forced] = split_str_once("abcdefghijklmnopqrstuvwxyz", 300);
		ASSERT_EQ(len, 26);
		ASSERT_EQ(pos, 0);
		ASSERT_EQ(forced, false);
	}

	// forced split
	{
		auto [len, pos, forced] = split_str_once("abcdefghijklmnopqrstuvwxyz", 80);
		ASSERT_EQ(len, 10);
		ASSERT_EQ(pos, 10);
		ASSERT_EQ(forced, true);
	}

	// Mantis #1152: always split on a newline even if the string would otherwise fit
	{
		auto [len, pos, forced] = split_str_once("abcde\n  fghij", 300);
		ASSERT_EQ(len, 5);
		ASSERT_EQ(pos, 8);
		ASSERT_EQ(forced, false);
	}

	// regular splits
	{
		auto [len, pos, forced] = split_str_once("abcde fghij klmno pqrstuvwxyz abcdefghijklmnopqrstuvwxyz", 300);
		ASSERT_EQ(len, 29);
		ASSERT_EQ(pos, 30);
		ASSERT_EQ(forced, false);
	}
	{
		auto [len, pos, forced] = split_str_once("abcdefghijklmnopqrstuvwxyz abcdefghijklmnopqrstuvwxyz", 300);
		ASSERT_EQ(len, 26);
		ASSERT_EQ(pos, 27);
		ASSERT_EQ(forced, false);
	}
	{
		auto [len, pos, forced] = split_str_once("abcdefghijklmnopqrstuvwxyz\t abcdefghijklmnopqrstuvwxyz", 300);
		ASSERT_EQ(len, 26);
		ASSERT_EQ(pos, 28);
		ASSERT_EQ(forced, false);
	}
	{
		auto [len, pos, forced] = split_str_once("abcdefghijklmnopqrstuvwxyz\n abcdefghijklmnopqrstuvwxyz", 300);
		ASSERT_EQ(len, 26);
		ASSERT_EQ(pos, 28);
		ASSERT_EQ(forced, false);
	}
	{
		auto [len, pos, forced] = split_str_once("abcdefghijklmnopqrstuvwxyz \nabcdefghijklmnopqrstuvwxyz", 300);
		ASSERT_EQ(len, 27);
		ASSERT_EQ(pos, 28);
		ASSERT_EQ(forced, false);
	}
}

TEST_F(SplitStrTest, split_str_once_advanced)
{
	// tricky splits
	{
		auto [len, pos, forced] = split_str_once("a\n\nbcde", 300);
		ASSERT_EQ(len, 1);
		ASSERT_EQ(pos, 2);
		ASSERT_EQ(forced, false);
	}
	{
		auto [len, pos, forced] = split_str_once("\n\nabcde", 300);
		ASSERT_EQ(len, 0);
		ASSERT_EQ(pos, 1);
		ASSERT_EQ(forced, false);
	}
	{
		auto [len, pos, forced] = split_str_once("abcd ", 300);
		ASSERT_EQ(len, 4);
		ASSERT_EQ(pos, 0);
		ASSERT_EQ(forced, false);
	}
	{
		auto [len, pos, forced] = split_str_once("abcde\t", 300);
		ASSERT_EQ(len, 5);
		ASSERT_EQ(pos, 0);
		ASSERT_EQ(forced, false);
	}
	{
		auto [len, pos, forced] = split_str_once("abc\n", 300);
		ASSERT_EQ(len, 3);
		ASSERT_EQ(pos, 0);
		ASSERT_EQ(forced, false);
	}
	{
		auto [len, pos, forced] = split_str_once("abcd\n ", 300);
		ASSERT_EQ(len, 4);
		ASSERT_EQ(pos, 0);
		ASSERT_EQ(forced, false);
	}
	{
		auto [len, pos, forced] = split_str_once("abcd \n", 300);
		ASSERT_EQ(len, 5);
		ASSERT_EQ(pos, 0);
		ASSERT_EQ(forced, false);
	}
	{
		auto [len, pos, forced] = split_str_once("aa", 16);
		ASSERT_EQ(len, 2);
		ASSERT_EQ(pos, 0);
		ASSERT_EQ(forced, false);
	}
	{
		auto [len, pos, forced] = split_str_once("aa ", 16);
		ASSERT_EQ(len, 2);
		ASSERT_EQ(pos, 0);
		ASSERT_EQ(forced, false);
	}
	{
		auto [len, pos, forced] = split_str_once("aa\n", 16);
		ASSERT_EQ(len, 2);
		ASSERT_EQ(pos, 0);
		ASSERT_EQ(forced, false);
	}

	// very tricky
	{
		auto [len, pos, forced] = split_str_once("\nabcdefghijklmnopqrstuvwxyz", 100);	// wide enough for the \n, but not wide enough for the whole string
		ASSERT_EQ(len, 0);
		ASSERT_EQ(pos, 1);
		ASSERT_EQ(forced, false);
	}

	// splits constrained by length
	{
		auto [len, pos, forced] = split_str_once("abcde fghij klmno pqrstuvwxyz abcdefghijklmnopqrstuvwxyz", 300, 15);
		ASSERT_EQ(len, 11);
		ASSERT_EQ(pos, 12);
		ASSERT_EQ(forced, false);
	}
	{
		auto [len, pos, forced] = split_str_once("abcde fghij klmno pqrstuvwxyz abcdefghijklmnopqrstuvwxyz", 300, 5);
		ASSERT_EQ(len, 5);
		ASSERT_EQ(pos, 6);
		ASSERT_EQ(forced, false);
	}
	{
		auto [len, pos, forced] = split_str_once("abcde fghij\nklmno pqrstuvwxyz abcdefghijklmnopqrstuvwxyz", 300, 11);
		ASSERT_EQ(len, 11);
		ASSERT_EQ(pos, 12);
		ASSERT_EQ(forced, false);
	}
	{
		auto [len, pos, forced] = split_str_once("abcdefghijklmnopqrstuvwxyz abcdefghijklmnopqrstuvwxyz", 300, 15);
		ASSERT_EQ(len, 15);
		ASSERT_EQ(pos, 15);
		ASSERT_EQ(forced, true);
	}
	{
		auto [len, pos, forced] = split_str_once("abcde", 300, 5);
		ASSERT_EQ(len, 5);
		ASSERT_EQ(pos, 0);
		ASSERT_EQ(forced, false);
	}
	{
		auto [len, pos, forced] = split_str_once("abcde ", 300, 5);
		ASSERT_EQ(len, 5);
		ASSERT_EQ(pos, 0);
		ASSERT_EQ(forced, false);
	}
	{
		auto [len, pos, forced] = split_str_once("abcde\n", 300, 5);
		ASSERT_EQ(len, 5);
		ASSERT_EQ(pos, 0);
		ASSERT_EQ(forced, false);
	}

	// measuring string width
	{
		int w;
		auto [len, pos, forced] = split_str_once("a b cde", 25, std::string::npos, 1.0f, &w);
		ASSERT_EQ(len, 3);
		ASSERT_EQ(pos, 4);
		ASSERT_EQ(forced, false);
		ASSERT_EQ(w, 24);
	}
	{
		int w;
		auto [len, pos, forced] = split_str_once("a      ", 25, std::string::npos, 1.0f, &w);
		ASSERT_EQ(len, 1);
		ASSERT_EQ(pos, 0);
		ASSERT_EQ(forced, false);
		ASSERT_EQ(w, 8);
	}

}
