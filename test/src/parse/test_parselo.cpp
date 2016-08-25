
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

	strcpy_s(test_str, "");
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

	test_str = "";
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

	strcpy_s(test_str, "");
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

	test_str = "";
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

	strcpy_s(test_str, "");
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

	test_str = "";
	drop_white_space(test_str);
	ASSERT_EQ(test_str, "");
}

TEST(ParseloTest, ignore_white_space) {
	ASSERT_EQ(EOF_CHAR, '\x80'); // for easier string building
	ASSERT_EQ(EOLN, '\x0A');

	const char test_str_reference[] = "A B  C  \x80\tD \tE\x0A F\x0A\x0A\x0D\x0A \t \n \t  GHI";

	char test_str[256];
	strcpy_s(test_str, test_str_reference);

	char * str = test_str;

	ignore_white_space(str);
	ASSERT_EQ(*str, 'A');
	ASSERT_STREQ(test_str, test_str_reference);

	str++;
	ignore_white_space(str);
	ASSERT_EQ(*str, 'B');
	ASSERT_STREQ(test_str, test_str_reference);

	str++;
	ignore_white_space(str);
	ASSERT_EQ(*str, 'C');
	ASSERT_STREQ(test_str, test_str_reference);

	// EOF_CHAR is not white space
	str++;
	ignore_white_space(str);
	ASSERT_EQ(*str, EOF_CHAR);
	ASSERT_STREQ(test_str, test_str_reference);

	str++;
	ignore_white_space(str);
	ASSERT_EQ(*str, 'D');
	ASSERT_STREQ(test_str, test_str_reference);

	str++;
	ignore_white_space(str);
	ASSERT_EQ(*str, 'E');
	ASSERT_STREQ(test_str, test_str_reference);

	str++;
	ignore_white_space(str);
	ASSERT_EQ(*str, 'F');
	ASSERT_STREQ(test_str, test_str_reference);

	// \r = \x0D is not white space
	str++;
	ignore_white_space(str);
	ASSERT_EQ(*str, '\x0D');
	ASSERT_STREQ(test_str, test_str_reference);

	str++;
	ignore_white_space(str);
	ASSERT_EQ(*str, 'G');
	ASSERT_STREQ(test_str, test_str_reference);

	ignore_white_space(str);
	ASSERT_EQ(*str, 'G');
	ASSERT_STREQ(test_str, test_str_reference);
}
