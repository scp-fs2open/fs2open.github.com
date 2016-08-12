
#include <gtest/gtest.h>

#include <parse/parselo.h>

TEST(ParseloTest, drop_trailing_whitespace_cstr) {
	char test_str[256];

	strcpy_s(test_str, "Test string       ");
	drop_trailing_white_space(test_str);
	ASSERT_STREQ(test_str, "Test string");

	strcpy_s(test_str, "Test string");
	drop_trailing_white_space(test_str);
	ASSERT_STREQ(test_str, "Test string");

	strcpy_s(test_str, "       ");
	drop_trailing_white_space(test_str);
	ASSERT_STREQ(test_str, "");
}

TEST(ParseloTest, drop_trailing_whitespace) {
	SCP_string test_str;

	test_str = "Test string       ";
	drop_trailing_white_space(test_str);
	ASSERT_EQ(test_str, "Test string");

	test_str = "Test string";
	drop_trailing_white_space(test_str);
	ASSERT_EQ(test_str, "Test string");

	test_str = "       ";
	drop_trailing_white_space(test_str);
	ASSERT_EQ(test_str, "");
}

TEST(ParseloTest, drop_leading_whitespace_cstr) {
	char test_str[256];

	strcpy_s(test_str, "          Test string");
	drop_leading_white_space(test_str);
	ASSERT_STREQ(test_str, "Test string");

	strcpy_s(test_str, "Test string");
	drop_leading_white_space(test_str);
	ASSERT_STREQ(test_str, "Test string");

	strcpy_s(test_str, "       ");
	drop_leading_white_space(test_str);
	ASSERT_STREQ(test_str, "");
}

TEST(ParseloTest, drop_leading_whitespace) {
	SCP_string test_str;

	test_str = "          Test string";
	drop_leading_white_space(test_str);
	ASSERT_EQ(test_str, "Test string");

	test_str = "Test string";
	drop_leading_white_space(test_str);
	ASSERT_EQ(test_str, "Test string");

	test_str = "       ";
	drop_leading_white_space(test_str);
	ASSERT_EQ(test_str, "");
}

TEST(ParseloTest, drop_whitespace_cstr) {
	char test_str[256];

	strcpy_s(test_str, "          Test string          ");
	drop_white_space(test_str);
	ASSERT_STREQ(test_str, "Test string");

	strcpy_s(test_str, "Test string          ");
	drop_white_space(test_str);
	ASSERT_STREQ(test_str, "Test string");

	strcpy_s(test_str, "              Test string");
	drop_white_space(test_str);
	ASSERT_STREQ(test_str, "Test string");

	strcpy_s(test_str, "Test string");
	drop_white_space(test_str);
	ASSERT_STREQ(test_str, "Test string");

	strcpy_s(test_str, "       ");
	drop_white_space(test_str);
	ASSERT_STREQ(test_str, "");
}

TEST(ParseloTest, drop_whitespace) {
	SCP_string test_str;

	test_str = "          Test string";
	drop_white_space(test_str);
	ASSERT_EQ(test_str, "Test string");

	test_str = "Test string                   ";
	drop_white_space(test_str);
	ASSERT_EQ(test_str, "Test string");

	test_str = "                       Test string";
	drop_white_space(test_str);
	ASSERT_EQ(test_str, "Test string");

	test_str = "Test string";
	drop_white_space(test_str);
	ASSERT_EQ(test_str, "Test string");

	test_str = "       ";
	drop_white_space(test_str);
	ASSERT_EQ(test_str, "");
}
