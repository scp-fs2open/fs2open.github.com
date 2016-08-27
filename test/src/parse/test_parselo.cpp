
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

TEST(ParseloTest, skip_to_string) {
	ASSERT_EQ(EOF_CHAR, '\x80'); // for easier string building
	ASSERT_EQ(EOLN, '\x0A');

	char test_str[256];
	char *str = test_str;
	int rc;

	strcpy_s(test_str, "\x80");
	str = test_str;
	rc = skip_to_string(str, "A");
	EXPECT_EQ(str, test_str);
	EXPECT_EQ(rc, 0);

	strcpy_s(test_str, "\x80");
	str = test_str;
	rc = skip_to_string(str, "A", "B");
	EXPECT_EQ(str, test_str);
	EXPECT_EQ(rc, 0);

	strcpy_s(test_str, "Test \x0A Test\x80");
	str = test_str;
	rc = skip_to_string(str, "A");
	EXPECT_STREQ(str, "\x80");
	EXPECT_EQ(rc, 0);

	strcpy_s(test_str, "Test \x0AX Test\x80");
	str = test_str;
	rc = skip_to_string(str, "X");
	EXPECT_STREQ(str, " Test\x80");
	EXPECT_EQ(rc, 1);

	// do not find search string if not at beginning of line (excluding whitespace)
	strcpy_s(test_str, "Test X\x0AX Test\x80");
	str = test_str;
	rc = skip_to_string(str, "X");
	EXPECT_STREQ(str, " Test\x80");
	EXPECT_EQ(rc, 1);

	strcpy_s(test_str, "Test XYZ \x0AX \x0AY \x0A XYZ \x0AXYZ Test\x80");
	str = test_str;
	rc = skip_to_string(str, "XYZ");
	EXPECT_STREQ(str, " \x0AXYZ Test\x80");
	EXPECT_EQ(rc, 1);

	// stop and return 0 if line begins with # but does not match to
	strcpy_s(test_str, "Test \x0A#X Y\x80");
	str = test_str;
	rc = skip_to_string(str, "#Z", "Y");
	EXPECT_STREQ(str, "#X Y\x80");
	EXPECT_EQ(rc, 0);

	strcpy_s(test_str, "Test \x0A#X Y\x80");
	str = test_str;
	rc = skip_to_string(str, "#X", "Y");
	EXPECT_STREQ(str, " Y\x80");
	EXPECT_EQ(rc, 1);

	strcpy_s(test_str, "Test \x0A Y \x0AX Test\x80");
	str = test_str;
	rc = skip_to_string(str, "X", "Y");
	EXPECT_STREQ(str, "Y \x0AX Test\x80");
	EXPECT_EQ(rc, -1);

	// end is recongized only at start of string (excluding whitespace)
	strcpy_s(test_str, "Test \x0A Test Y \x0AX Test\x80");
	str = test_str;
	rc = skip_to_string(str, "X", "Y");
	EXPECT_STREQ(str, " Test\x80");
	EXPECT_EQ(rc, 1);
}

TEST(ParseloTest, skip_to_start_of_string) {
	ASSERT_EQ(EOF_CHAR, '\x80'); // for easier string building
	ASSERT_EQ(EOLN, '\x0A');

	char test_str[256];
	char *str = test_str;
	int rc;

	strcpy_s(test_str, "\x80");
	str = test_str;
	rc = skip_to_start_of_string(str, "A");
	EXPECT_EQ(str, test_str);
	EXPECT_EQ(rc, 0);

	strcpy_s(test_str, "\x80");
	str = test_str;
	rc = skip_to_start_of_string(str, "A", "B");
	EXPECT_EQ(str, test_str);
	EXPECT_EQ(rc, 0);

	strcpy_s(test_str, "Test \x0A Test\x80");
	str = test_str;
	rc = skip_to_start_of_string(str, "A");
	EXPECT_STREQ(str, "\x80");
	EXPECT_EQ(rc, 0);

	strcpy_s(test_str, "Test \x0AX Test\x80");
	str = test_str;
	rc = skip_to_start_of_string(str, "X");
	EXPECT_STREQ(str, "X Test\x80");
	EXPECT_EQ(rc, 1);

	// do not find search string if not at beginning of line (excluding whitespace)
	strcpy_s(test_str, "Test X\x0AX Test\x80");
	str = test_str;
	rc = skip_to_start_of_string(str, "X");
	EXPECT_STREQ(str, "X Test\x80");
	EXPECT_EQ(rc, 1);

	strcpy_s(test_str, "Test XYZ \x0AX \x0AY \x0A XYZ \x0AXYZ Test\x80");
	str = test_str;
	rc = skip_to_start_of_string(str, "XYZ");
	EXPECT_STREQ(str, "XYZ \x0AXYZ Test\x80");
	EXPECT_EQ(rc, 1);

	// stop and return 0 if line begins with # but does not match to
	strcpy_s(test_str, "Test \x0A#X Y\x80");
	str = test_str;
	rc = skip_to_start_of_string(str, "#Z", "Y");
	EXPECT_STREQ(str, "#X Y\x80");
	EXPECT_EQ(rc, 0);

	strcpy_s(test_str, "Test \x0A#X Y\x80");
	str = test_str;
	rc = skip_to_start_of_string(str, "#X", "Y");
	EXPECT_STREQ(str, "#X Y\x80");
	EXPECT_EQ(rc, 1);

	strcpy_s(test_str, "Test \x0A Y \x0AX Test\x80");
	str = test_str;
	rc = skip_to_start_of_string(str, "X", "Y");
	EXPECT_STREQ(str, "Y \x0AX Test\x80");
	EXPECT_EQ(rc, 0);

	// end is recongized only at start of string (excluding whitespace)
	strcpy_s(test_str, "Test \x0A Test Y \x0AX Test\x80");
	str = test_str;
	rc = skip_to_start_of_string(str, "X", "Y");
	EXPECT_STREQ(str, "X Test\x80");
	EXPECT_EQ(rc, 1);
}

TEST(ParseloTest, skip_to_start_of_string_either) {
	ASSERT_EQ(EOF_CHAR, '\x80'); // for easier string building
	ASSERT_EQ(EOLN, '\x0A');

	char test_str[256];
	char *str = test_str;
	int rc;

	strcpy_s(test_str, "\x80");
	str = test_str;
	rc = skip_to_start_of_string_either(str, "A", "B");
	EXPECT_EQ(str, test_str);
	EXPECT_EQ(rc, 0);

	strcpy_s(test_str, "\x80");
	str = test_str;
	rc = skip_to_start_of_string_either(str, "A", "B", "C");
	EXPECT_EQ(str, test_str);
	EXPECT_EQ(rc, 0);

	strcpy_s(test_str, "Test \x0A Test\x80");
	str = test_str;
	rc = skip_to_start_of_string_either(str, "A", "B");
	EXPECT_STREQ(str, "\x80");
	EXPECT_EQ(rc, 0);

	strcpy_s(test_str, "Test \x0AX Test\x80");
	str = test_str;
	rc = skip_to_start_of_string_either(str, "X", "Y");
	EXPECT_STREQ(str, "X Test\x80");
	EXPECT_EQ(rc, 1);

	strcpy_s(test_str, "Test \x0AY Test\x80");
	str = test_str;
	rc = skip_to_start_of_string_either(str, "X", "Y");
	EXPECT_STREQ(str, "Y Test\x80");
	EXPECT_EQ(rc, 1);

	strcpy_s(test_str, "Test \x0AX \x0AY Test\x80");
	str = test_str;
	rc = skip_to_start_of_string_either(str, "X", "Y");
	EXPECT_STREQ(str, "X \x0AY Test\x80");
	EXPECT_EQ(rc, 1);

	strcpy_s(test_str, "Test \x0AY \x0AX Test\x80");
	str = test_str;
	rc = skip_to_start_of_string_either(str, "X", "Y");
	EXPECT_STREQ(str, "Y \x0AX Test\x80");
	EXPECT_EQ(rc, 1);

	// do not find search string if not at beginning of line (excluding whitespace)
	strcpy_s(test_str, "Test X\x0AX Test\x80");
	str = test_str;
	rc = skip_to_start_of_string_either(str, "X", "Y", "Z");
	EXPECT_STREQ(str, "X Test\x80");
	EXPECT_EQ(rc, 1);

	strcpy_s(test_str, "Test Y\x0AX Test\x80");
	str = test_str;
	rc = skip_to_start_of_string_either(str, "X", "Y");
	EXPECT_STREQ(str, "X Test\x80");
	EXPECT_EQ(rc, 1);

	strcpy_s(test_str, "Test XYZ \x0AX \x0A A \x0A B \x0A AB \x0AY \x0A XYZ \x0AXYZ Test\x80");
	str = test_str;
	rc = skip_to_start_of_string_either(str, "XYZ", "ABC");
	EXPECT_STREQ(str, "XYZ \x0AXYZ Test\x80");
	EXPECT_EQ(rc, 1);

	strcpy_s(test_str, "Test ABC \x0AX \x0A A \x0A B \x0A AB \x0AY \x0A ABC \x0AXYZ Test\x80");
	str = test_str;
	rc = skip_to_start_of_string_either(str, "XYZ", "ABC");
	EXPECT_STREQ(str, "ABC \x0AXYZ Test\x80");
	EXPECT_EQ(rc, 1);

	// stop and return 0 if line begins with # but does not match either string and end exists
	strcpy_s(test_str, "Test \x0A#X Y\x80");
	str = test_str;
	rc = skip_to_start_of_string_either(str, "#Z", "Y", "FOO");
	EXPECT_STREQ(str, "#X Y\x80");
	EXPECT_EQ(rc, 0);

	strcpy_s(test_str, "Test \x0A#X Y\x80");
	str = test_str;
	rc = skip_to_start_of_string_either(str, "Z", "#Y", "FOO");
	EXPECT_STREQ(str, "#X Y\x80");
	EXPECT_EQ(rc, 0);

	strcpy_s(test_str, "Test \x0A#X #Y\x80");
	str = test_str;
	rc = skip_to_start_of_string_either(str, "#Z", "#Y", "FOO");
	EXPECT_STREQ(str, "#X #Y\x80");
	EXPECT_EQ(rc, 0);

	strcpy_s(test_str, "Test \x0A#X \x0A#Y\x80");
	str = test_str;
	rc = skip_to_start_of_string_either(str, "#X", "#Y", "FOO");
	EXPECT_STREQ(str, "#X \x0A#Y\x80");
	EXPECT_EQ(rc, 1);

	strcpy_s(test_str, "Test \x0A#Y \x0A#X\x80");
	str = test_str;
	rc = skip_to_start_of_string_either(str, "#X", "#Y", "FOO");
	EXPECT_STREQ(str, "#Y \x0A#X\x80");
	EXPECT_EQ(rc, 1);

	strcpy_s(test_str, "Test \x0A#Y \x0A#Y\x80");
	str = test_str;
	rc = skip_to_start_of_string_either(str, "#X", "#Y", "FOO");
	EXPECT_STREQ(str, "#Y \x0A#Y\x80");
	EXPECT_EQ(rc, 1);

	strcpy_s(test_str, "Test \x0A FOO\x0A#X #Y\x80");
	str = test_str;
	rc = skip_to_start_of_string_either(str, "#Z", "#Y", "FOO");
	EXPECT_STREQ(str, "FOO\x0A#X #Y\x80");
	EXPECT_EQ(rc, 0);

	strcpy_s(test_str, "Test \x0A Y FOO\x0AX Test\x80");
	str = test_str;
	rc = skip_to_start_of_string_either(str, "X", "Y", "FOO");
	EXPECT_STREQ(str, "Y FOO\x0AX Test\x80");
	EXPECT_EQ(rc, 1);

	// end is recongized only at start of string (excluding whitespace)
	strcpy_s(test_str, "Test \x0A Test FOO \x0AX Test\x80");
	str = test_str;
	rc = skip_to_start_of_string_either(str, "X", "Y", "FOO");
	EXPECT_STREQ(str, "X Test\x80");
	EXPECT_EQ(rc, 1);
}
