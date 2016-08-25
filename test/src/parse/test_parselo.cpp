
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

TEST(ParseloTest, ignore_gray_space) {
	ASSERT_EQ(EOF_CHAR, '\x80'); // for easier string building
	ASSERT_EQ(EOLN, '\x0A');

	const char test_str_reference[] = "A B  C  \x80\tD \tE\x0A F\x0D\x0A\x0A";

	char test_str[256];
	strcpy_s(test_str, test_str_reference);

	char * str = test_str;

	ignore_gray_space(str);
	ASSERT_EQ(*str, 'A');
	ASSERT_STREQ(test_str, test_str_reference);

	str++;
	ignore_gray_space(str);
	ASSERT_EQ(*str, 'B');
	ASSERT_STREQ(test_str, test_str_reference);

	str++;
	ignore_gray_space(str);
	ASSERT_EQ(*str, 'C');
	ASSERT_STREQ(test_str, test_str_reference);

	// EOF_CHAR is not gray space
	str++;
	ignore_gray_space(str);
	ASSERT_EQ(*str, EOF_CHAR);
	ASSERT_STREQ(test_str, test_str_reference);

	str++;
	ignore_gray_space(str);
	ASSERT_EQ(*str, 'D');
	ASSERT_STREQ(test_str, test_str_reference);

	str++;
	ignore_gray_space(str);
	ASSERT_EQ(*str, 'E');
	ASSERT_STREQ(test_str, test_str_reference);

	str++;
	ignore_gray_space(str);
	ASSERT_EQ(*str, EOLN);
	ASSERT_STREQ(test_str, test_str_reference);

	ignore_gray_space(str);
	ASSERT_EQ(*str, EOLN);
	ASSERT_STREQ(test_str, test_str_reference);

	str++;
	ignore_gray_space(str);
	ASSERT_EQ(*str, 'F');
	ASSERT_STREQ(test_str, test_str_reference);

	// \r = \x0D is not gray space
	str++;
	ignore_gray_space(str);
	ASSERT_EQ(*str, '\x0D');
	ASSERT_STREQ(test_str, test_str_reference);

	str++;
	ignore_gray_space(str);
	ASSERT_EQ(*str, EOLN);
	ASSERT_STREQ(test_str, test_str_reference);

	str++;
	ignore_gray_space(str);
	ASSERT_EQ(*str, EOLN);
	ASSERT_STREQ(test_str, test_str_reference);
}

TEST(ParseloTest, skip_token) {
	ASSERT_EQ(EOF_CHAR, '\x80'); // for easier string building
	ASSERT_EQ(EOLN, '\x0A');

	char test_str[256];
	char *str = test_str;

	strcpy_s(test_str, " A$C\x80");
	str = test_str;
	skip_token(str);
	ASSERT_STREQ(str, "\x80");

	strcpy_s(test_str, " A(B$C\t \x80");
	str = test_str;
	skip_token(str);
	ASSERT_STREQ(str, "\t \x80");

	strcpy_s(test_str, " \t  #ABC D\x80");
	str = test_str;
	skip_token(str);
	ASSERT_STREQ(str, " D\x80");

	strcpy_s(test_str, "\n\t A\"BC\x80 D");
	str = test_str;
	skip_token(str);
	ASSERT_STREQ(str, "\x80 D");

	strcpy_s(test_str, " \t \x0A \x80");
	str = test_str;
	skip_token(str);
	ASSERT_STREQ(str, "\x80");

	strcpy_s(test_str, "\x80");
	str = test_str;
	skip_token(str);
	ASSERT_STREQ(str, "\x80");
}

TEST(ParseloTest, advance_to_eoln) {
	ASSERT_EQ(EOF_CHAR, '\x80'); // for easier string building
	ASSERT_EQ(EOLN, '\x0A');

	char test_str[256];
	char *str = test_str;

	strcpy_s(test_str, "  \x0A");
	str = test_str;
	advance_to_eoln(str);
	ASSERT_STREQ(str, "\x0A");

	strcpy_s(test_str, "  \x80");
	str = test_str;
	advance_to_eoln(str);
	ASSERT_STREQ(str, "\x80");

	strcpy_s(test_str, " \x0A \x0A\x80");
	str = test_str;
	advance_to_eoln(str);
	ASSERT_STREQ(str, "\x0A \x0A\x80");

	strcpy_s(test_str, " $Text\x0A");
	str = test_str;
	advance_to_eoln(str);
	ASSERT_STREQ(str, "\x0A");

	strcpy_s(test_str, " $Text\x80");
	str = test_str;
	advance_to_eoln(str);
	ASSERT_STREQ(str, "\x80");
}

TEST(ParseloTest, advance_to_terminator) {
	ASSERT_EQ(EOF_CHAR, '\x80'); // for easier string building
	ASSERT_EQ(EOLN, '\x0A');

	char test_str[256];
	char *str = test_str;

	// the first part is just like advance_to_eoln
	strcpy_s(test_str, "  \x0A");
	str = test_str;
	advance_to_terminator(str, NULL);
	ASSERT_STREQ(str, "\x0A");

	strcpy_s(test_str, "  \x0A");
	str = test_str;
	advance_to_terminator(str, "");
	ASSERT_STREQ(str, "\x0A");

	strcpy_s(test_str, "  \x80");
	str = test_str;
	advance_to_terminator(str, NULL);
	ASSERT_STREQ(str, "\x80");

	strcpy_s(test_str, " \x0A \x0A\x80");
	str = test_str;
	advance_to_terminator(str, NULL);
	ASSERT_STREQ(str, "\x0A \x0A\x80");

	strcpy_s(test_str, " $Text\x0A");
	str = test_str;
	advance_to_terminator(str, NULL);
	ASSERT_STREQ(str, "\x0A");

	// the rest (with more parameters) is different
	strcpy_s(test_str, " $Text\x80");
	str = test_str;
	advance_to_terminator(str, NULL);
	ASSERT_STREQ(str, "\x80");
	strcpy_s(test_str, "  \x0A");
	str = test_str;
	advance_to_terminator(str, "$");
	ASSERT_STREQ(str, "\x0A");

	strcpy_s(test_str, "  \x0A#");
	str = test_str;
	advance_to_terminator(str, "#");
	ASSERT_STREQ(str, "\x0A#");

	strcpy_s(test_str, ",  \x0A");
	str = test_str;
	advance_to_terminator(str, ",");
	ASSERT_STREQ(str, ",  \x0A");

	strcpy_s(test_str, "Text,  \x0A");
	str = test_str;
	advance_to_terminator(str, ",");
	ASSERT_STREQ(str, ",  \x0A");

	strcpy_s(test_str, "Text& .Text");
	str = test_str;
	advance_to_terminator(str, "& .");
	ASSERT_STREQ(str, "& .Text");

	strcpy_s(test_str, "Text .&Text");
	str = test_str;
	advance_to_terminator(str, "& .");
	ASSERT_STREQ(str, " .&Text");

	strcpy_s(test_str, "Text.& Text");
	str = test_str;
	advance_to_terminator(str, "& .");
	ASSERT_STREQ(str, ".& Text");
}
