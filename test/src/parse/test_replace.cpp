
#include <gtest/gtest.h>

#include <parse/parselo.h>

#include "globalincs/safe_strings.h"

#include "util/FSTestFixture.h"

class ReplaceTest : public test::FSTestFixture {
 public:
	ReplaceTest() : test::FSTestFixture(INIT_MOD_TABLE | INIT_CFILE) {
		pushModDir("parselo");
	}

 protected:
	void SetUp() override {
		test::FSTestFixture::SetUp();
	}
	void TearDown() override {
		stop_parse();

		test::FSTestFixture::TearDown();
	}
};

TEST_F(ReplaceTest, replace_one_cstr)
{
	constexpr int MAX_LEN = 10;
	char buf[MAX_LEN + 1];
	strcpy_s(buf, "aabbccddee");

	// search string not found
	ASSERT_EQ(-1, replace_one(buf, "ff", "XX", MAX_LEN));
	ASSERT_EQ(-1, replace_one(buf, "dd", "XX", MAX_LEN, 5));

	// exceed max MAX_LENgth
	ASSERT_EQ(-2, replace_one(buf, "dd", "XXX", MAX_LEN));

	// bad args
	ASSERT_EQ(-3, replace_one(nullptr, "aa", "bb", MAX_LEN));
	ASSERT_EQ(-3, replace_one(buf, nullptr, "bb", MAX_LEN));
	ASSERT_EQ(-3, replace_one(buf, "aa", nullptr, MAX_LEN));

	// successful replacement at end
	ASSERT_EQ(8, replace_one(buf, "ee", "fe", MAX_LEN));
	ASSERT_STREQ("aabbccddfe", buf);
	ASSERT_EQ(9, replace_one(buf, "e", "f", MAX_LEN));
	ASSERT_STREQ("aabbccddff", buf);

	// successful single replacement
	ASSERT_EQ(8, replace_one(buf, "f", "g", MAX_LEN));
	ASSERT_STREQ("aabbccddgf", buf);
	ASSERT_EQ(2, replace_one(buf, "b", "h", MAX_LEN));
	ASSERT_STREQ("aahbccddgf", buf);

	// successful replacement at beginning
	ASSERT_EQ(0, replace_one(buf, "a", "b", MAX_LEN));
	ASSERT_STREQ("bahbccddgf", buf);
	ASSERT_EQ(0, replace_one(buf, "bah", "gak", MAX_LEN));
	ASSERT_STREQ("gakbccddgf", buf);

	// successful replacement when growing string
	strcpy_s(buf, "abcd");
	ASSERT_EQ(2, replace_one(buf, "cd", "xyz123", MAX_LEN));
	ASSERT_STREQ("abxyz123", buf);
	ASSERT_EQ(5, replace_one(buf, "123", "45678", MAX_LEN));
	ASSERT_STREQ("abxyz45678", buf);

	// successful replacement of entire string
	ASSERT_EQ(0, replace_one(buf, "abxyz45678", "0123456789", MAX_LEN));
	ASSERT_STREQ("0123456789", buf);
}

TEST_F(ReplaceTest, replace_one_scpstr)
{
	SCP_string str = "aabbccddee";

	// search string not found
	ASSERT_EQ(-1, replace_one(str, "ff", "XX"));
	// SCP_string version of replace_one does not have a range argument

	// successful replacement at end
	ASSERT_EQ(8, replace_one(str, "ee", "fe"));
	ASSERT_STREQ("aabbccddfe", str.c_str());
	ASSERT_EQ(9, replace_one(str, "e", "f"));
	ASSERT_STREQ("aabbccddff", str.c_str());

	// successful single replacement
	ASSERT_EQ(8, replace_one(str, "f", "g"));
	ASSERT_STREQ("aabbccddgf", str.c_str());
	ASSERT_EQ(2, replace_one(str, "b", "h"));
	ASSERT_STREQ("aahbccddgf", str.c_str());

	// successful replacement at beginning
	ASSERT_EQ(0, replace_one(str, "a", "b"));
	ASSERT_STREQ("bahbccddgf", str.c_str());
	ASSERT_EQ(0, replace_one(str, "bah", "gak"));
	ASSERT_STREQ("gakbccddgf", str.c_str());

	// successful replacement when growing string
	str = "abcd";
	ASSERT_EQ(2, replace_one(str, "cd", "xyz123"));
	ASSERT_STREQ("abxyz123", str.c_str());
	ASSERT_EQ(5, replace_one(str, "123", "45678"));
	ASSERT_STREQ("abxyz45678", str.c_str());

	// successful replacement of entire string
	ASSERT_EQ(0, replace_one(str, "abxyz45678", "0123456789"));
	ASSERT_STREQ("0123456789", str.c_str());
}

TEST_F(ReplaceTest, replace_all_cstr)
{
	constexpr int MAX_LEN = 10;
	char buf[MAX_LEN + 1];
	strcpy_s(buf, "ababcdcdef");

	// search string not found
	ASSERT_EQ(0, replace_all(buf, "gg", "XX", MAX_LEN));
	ASSERT_EQ(0, replace_all(buf, "cd", "XX", MAX_LEN, 3));

	// exceed max MAX_LENgth
	ASSERT_EQ(-2, replace_all(buf, "ef", "XXX", MAX_LEN));
	strcpy_s(buf, "abababab");
	ASSERT_EQ(-2, replace_all(buf, "ab", "mno", MAX_LEN));
	ASSERT_STREQ("mnomnoabab", buf);
	strcpy_s(buf, "ababcdcdef");

	// successful single replacement
	ASSERT_EQ(1, replace_all(buf, "ef", "jk", MAX_LEN));
	ASSERT_STREQ("ababcdcdjk", buf);

	// successful double replacement
	ASSERT_EQ(2, replace_all(buf, "ab", "pq", MAX_LEN));
	ASSERT_STREQ("pqpqcdcdjk", buf);

	// successful replacement when growing string
	strcpy_s(buf, "abcdcd");
	ASSERT_EQ(2, replace_all(buf, "cd", "xyz", MAX_LEN));
	ASSERT_STREQ("abxyzxyz", buf);
	strcpy_s(buf, "abbacdcd");
	ASSERT_EQ(2, replace_all(buf, "cd", "xyz", MAX_LEN));
	ASSERT_STREQ("abbaxyzxyz", buf);
}

TEST_F(ReplaceTest, replace_all_scpstr)
{
	SCP_string str = "ababcdcdef";

	// search string not found
	ASSERT_EQ(0, replace_all(str, "gg", "XX"));
	// SCP_string version of replace_all does not have a range argument

	// successful single replacement
	ASSERT_EQ(1, replace_all(str, "ef", "jk"));
	ASSERT_STREQ("ababcdcdjk", str.c_str());

	// successful double replacement
	ASSERT_EQ(2, replace_all(str, "ab", "pq"));
	ASSERT_STREQ("pqpqcdcdjk", str.c_str());

	// successful replacement when growing string
	str = "abcdcd";
	ASSERT_EQ(2, replace_all(str, "cd", "xyz"));
	ASSERT_STREQ("abxyzxyz", str.c_str());
	str = "abbacdcd";
	ASSERT_EQ(2, replace_all(str, "cd", "xyz"));
	ASSERT_STREQ("abbaxyzxyz", str.c_str());
}
