
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
	constexpr int len = 10;
	char buf[len + 1];
	strcpy_s(buf, "aabbccddee");

	// search string not found
	ASSERT_EQ(0, replace_one(buf, "ff", "XX", len));
	ASSERT_EQ(0, replace_one(buf, "dd", "XX", len, 5));

	// exceed max length
	ASSERT_EQ(-1, replace_one(buf, "dd", "XXX", len));

	// successful replacement at end
	ASSERT_EQ(8, replace_one(buf, "ee", "fe", len));
	ASSERT_STREQ("aabbccddfe", buf);
	ASSERT_EQ(9, replace_one(buf, "e", "f", len));
	ASSERT_STREQ("aabbccddff", buf);

	// successful single replacement
	ASSERT_EQ(8, replace_one(buf, "f", "g", len));
	ASSERT_STREQ("aabbccddgf", buf);
	ASSERT_EQ(2, replace_one(buf, "b", "h", len));
	ASSERT_STREQ("aahbccddgf", buf);

	// successful replacement at beginning
	ASSERT_EQ(0, replace_one(buf, "a", "b", len));
	ASSERT_STREQ("bahbccddgf", buf);
	ASSERT_EQ(0, replace_one(buf, "bah", "gak", len));
	ASSERT_STREQ("gakbccddgf", buf);

	// successful replacement when growing string
	strcpy_s(buf, "abcd");
	ASSERT_EQ(2, replace_one(buf, "cd", "xyz123", len));
	ASSERT_STREQ("abxyz123", buf);
	ASSERT_EQ(5, replace_one(buf, "123", "45678", len));
	ASSERT_STREQ("abxyz45678", buf);

	// successful replacement of entire string
	ASSERT_EQ(0, replace_one(buf, "abxyz45678", "0123456789", len));
	ASSERT_STREQ("0123456789", buf);
}

TEST_F(ReplaceTest, replace_one_scpstr)
{
	SCP_string buf = "aabbccddee";

	// search string not found
	ASSERT_EQ(buf, replace_one(buf, "ff", "XX"));
	// SCP_string version of replace_one does not have a range argument

	// successful replacement at end
	ASSERT_EQ("aabbccddfe", replace_one(buf, "ee", "fe"));
	ASSERT_EQ("aabbccddff", replace_one(buf, "e", "f"));

	// successful single replacement
	ASSERT_EQ("aabbccddgf", replace_one(buf, "f", "g"));
	ASSERT_EQ("aahbccddgf", replace_one(buf, "b", "h"));

	// successful replacement at beginning
	ASSERT_EQ("bahbccddgf", replace_one(buf, "a", "b"));
	ASSERT_EQ("gakbccddgf", replace_one(buf, "bah", "gak"));

	// successful replacement when growing string
	buf = "abcd";
	ASSERT_EQ("abxyz123", replace_one(buf, "cd", "xyz123"));
	ASSERT_EQ("abxyz45678", replace_one(buf, "123", "45678"));

	// successful replacement of entire string
	ASSERT_EQ("0123456789", replace_one(buf, "abxyz45678", "0123456789"));
}

TEST_F(ReplaceTest, replace_all_cstr)
{
	constexpr int len = 10;
	char buf[len + 1];
	strcpy_s(buf, "ababcdcdef");

	// search string not found
	ASSERT_EQ(0, replace_all(buf, "gg", "XX", len));
	ASSERT_EQ(0, replace_all(buf, "cd", "XX", len, 3));

	// exceed max length
	ASSERT_EQ(-1, replace_all(buf, "ef", "XXX", len));
	strcpy_s(buf, "abababab");
	ASSERT_EQ(-1, replace_all(buf, "ab", "mno", len));
	ASSERT_STREQ("mnoababab", buf);
	strcpy_s(buf, "ababcdcdef");

	// successful single replacement
	ASSERT_EQ(1, replace_all(buf, "ef", "jk", len));
	ASSERT_STREQ("ababcdcdjk", buf);

	// successful double replacement
	ASSERT_EQ(2, replace_all(buf, "ab", "pq", len));
	ASSERT_STREQ("pqpqcdcdjk", buf);

	// successful replacement when growing string
	strcpy_s(buf, "abcdcd");
	ASSERT_EQ(2, replace_all(buf, "cd", "xyz", len));
	ASSERT_STREQ("abxyzxyz", buf);
	strcpy_s(buf, "abbacdcd");
	ASSERT_EQ(2, replace_all(buf, "cd", "xyz", len));
	ASSERT_STREQ("abbaxyzxyz", buf);
}

TEST_F(ReplaceTest, replace_all_scpstr)
{
	SCP_string buf = "ababcdcdef";

	// search string not found
	ASSERT_EQ(buf, replace_all(buf, "gg", "XX"));
	// SCP_string version of replace_all does not have a range argument

	// successful single replacement
	ASSERT_EQ("ababcdcdjk", replace_all(buf, "ef", "jk"));

	// successful double replacement
	ASSERT_EQ("pqpqcdcdjk", replace_all(buf, "ab", "pq"));

	// successful replacement when growing string
	buf = "abcdcd";
	ASSERT_EQ("abxyzxyz", replace_all(buf, "cd", "xyz"));
	buf = "abbacdcd";
	ASSERT_EQ("abbaxyzxyz", replace_all(buf, "cd", "xyz"));
}
