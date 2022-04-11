
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
	ASSERT_EQ(-1, replace_one(buf, "ff", "XX", len));
	ASSERT_EQ(-1, replace_one(buf, "dd", "XX", len, 5));

	// exceed max length
	ASSERT_EQ(-2, replace_one(buf, "dd", "XXX", len));

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
	ASSERT_EQ(-1, replace_one(buf, "ff", "XX"));
	// SCP_string version of replace_one does not have a range argument

	// successful replacement at end
	ASSERT_EQ(8, replace_one(buf, "ee", "fe"));
	ASSERT_STREQ("aabbccddfe", buf.c_str());
	ASSERT_EQ(9, replace_one(buf, "e", "f"));
	ASSERT_STREQ("aabbccddff", buf.c_str());

	// successful single replacement
	ASSERT_EQ(8, replace_one(buf, "f", "g"));
	ASSERT_STREQ("aabbccddgf", buf.c_str());
	ASSERT_EQ(2, replace_one(buf, "b", "h"));
	ASSERT_STREQ("aahbccddgf", buf.c_str());

	// successful replacement at beginning
	ASSERT_EQ(0, replace_one(buf, "a", "b"));
	ASSERT_STREQ("bahbccddgf", buf.c_str());
	ASSERT_EQ(0, replace_one(buf, "bah", "gak"));
	ASSERT_STREQ("gakbccddgf", buf.c_str());

	// successful replacement when growing string
	buf = "abcd";
	ASSERT_EQ(2, replace_one(buf, "cd", "xyz123"));
	ASSERT_STREQ("abxyz123", buf.c_str());
	ASSERT_EQ(5, replace_one(buf, "123", "45678"));
	ASSERT_STREQ("abxyz45678", buf.c_str());

	// successful replacement of entire string
	ASSERT_EQ(0, replace_one(buf, "abxyz45678", "0123456789"));
	ASSERT_STREQ("0123456789", buf.c_str());
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
	ASSERT_EQ(-2, replace_all(buf, "ef", "XXX", len));
	strcpy_s(buf, "abababab");
	ASSERT_EQ(-2, replace_all(buf, "ab", "mno", len));
	ASSERT_STREQ("mnomnoabab", buf);
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
	ASSERT_EQ(0, replace_all(buf, "gg", "XX"));
	// SCP_string version of replace_all does not have a range argument

	// successful single replacement
	ASSERT_EQ(1, replace_all(buf, "ef", "jk"));
	ASSERT_STREQ("ababcdcdjk", buf.c_str());

	// successful double replacement
	ASSERT_EQ(2, replace_all(buf, "ab", "pq"));
	ASSERT_STREQ("pqpqcdcdjk", buf.c_str());

	// successful replacement when growing string
	buf = "abcdcd";
	ASSERT_EQ(2, replace_all(buf, "cd", "xyz"));
	ASSERT_STREQ("abxyzxyz", buf.c_str());
	buf = "abbacdcd";
	ASSERT_EQ(2, replace_all(buf, "cd", "xyz"));
	ASSERT_STREQ("abbaxyzxyz", buf.c_str());
}
