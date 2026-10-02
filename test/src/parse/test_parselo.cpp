
#include <gtest/gtest.h>

#include <parse/parselo.h>

#include "util/FSTestFixture.h"

class ParseloTest : public test::FSTestFixture {
 public:
	ParseloTest() : test::FSTestFixture(INIT_MOD_TABLE | INIT_CFILE) {
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

TEST_F(ParseloTest, parse_pausing) {
	read_file_text("test.tbl", CF_TYPE_TABLES);
	reset_parse();

	// Consume a token
	required_string("#Start");

	// Read the nested file into our own buffers so the outer parse is not disturbed
	char file_text[1024];
	char file_text_raw[1024];

	memset(file_text, 0, sizeof(file_text));
	memset(file_text_raw, 0, sizeof(file_text_raw));

	read_file_text("test2.tbl", CF_TYPE_TABLES, file_text, file_text_raw);	// NOLINT(readability-suspicious-call-argument)

	{
		// Now pause parsing and check if the right text is loaded
		PauseParseGuard guard(file_text, "test2.tbl");

		// Line numbers should be relative to the nested file, which requires
		// Parse_text to track the buffer containing Mp
		required_string("#Begin");
		EXPECT_EQ(get_line_num(), 1);
		required_string("#End");
		EXPECT_EQ(get_line_num(), 2);
	}

	// We should be back in the original file
	required_string("$Token:");
	EXPECT_EQ(get_line_num(), 3);
	required_string("+OtherToken:");
	required_string("#End");
}

TEST_F(ParseloTest, parse_pausing_exception_restores_state) {
	read_file_text("test.tbl", CF_TYPE_TABLES);
	reset_parse();

	required_string("#Start");

	char file_text[1024];
	char file_text_raw[1024];

	memset(file_text, 0, sizeof(file_text));
	memset(file_text_raw, 0, sizeof(file_text_raw));

	read_file_text("test2.tbl", CF_TYPE_TABLES, file_text, file_text_raw);	// NOLINT(readability-suspicious-call-argument)

	bool caught = false;
	try {
		PauseParseGuard guard(file_text, "test2.tbl");
		required_string("#Begin");
		throw parse::ParseException("simulated parse failure");
	} catch (const parse::ParseException&) {
		caught = true;
	}
	ASSERT_TRUE(caught);

	// The guard's destructor should have restored the outer file during stack unwinding
	required_string("$Token:");
	required_string("+OtherToken:");
	required_string("#End");
}

TEST_F(ParseloTest, parse_pausing_early_unpause) {
	read_file_text("test.tbl", CF_TYPE_TABLES);
	reset_parse();

	required_string("#Start");

	{
		// Bookmark the current position for lookahead
		PauseParseGuard guard(Mp, Current_filename);
		required_string("$Token:");

		// Rewind to the bookmarked position
		guard.unpause();

		// A second unpause is a silent no-op
		guard.unpause();
	}	// ...and so is the destructor

	// The lookahead should have been rewound, so the token is still pending
	required_string("$Token:");
	required_string("+OtherToken:");
	required_string("#End");
}

TEST_F(ParseloTest, utf8_with_bom) {
	read_file_text("bom_test.tbl", CF_TYPE_TABLES);
	reset_parse();

	SCP_string content;
	stuff_string(content, F_NAME);

	ASSERT_STREQ(content.c_str(), "Hello World");
}

TEST(ParseloUtilTest, drop_trailing_whitespace_cstr) {
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

TEST(ParseloUtilTest, drop_trailing_whitespace) {
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

TEST(ParseloUtilTest, drop_leading_whitespace_cstr) {
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

TEST(ParseloUtilTest, drop_leading_whitespace) {
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

TEST(ParseloUtilTest, drop_whitespace_cstr) {
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

TEST(ParseloUtilTest, drop_whitespace) {
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

