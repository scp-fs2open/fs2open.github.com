
#include <gtest/gtest.h>

#include <parse/parselo.h>

#include "util/FSTestFixture.h"

class ParseloTest : public test::FSTestFixture {
 public:
	ParseloTest() : test::FSTestFixture(INIT_MOD_TABLE | INIT_CFILE) {
		pushModDir("parselo");
	}

 protected:
	virtual void SetUp() override {
		test::FSTestFixture::SetUp();
	}
	virtual void TearDown() override {
		stop_parse();

		test::FSTestFixture::TearDown();
	}
};

TEST_F(ParseloTest, parse_pausing) {
	read_file_text("test.tbl", CF_TYPE_TABLES);
	reset_parse();

	// Consume a token
	required_string("#Start");

	// Now pause parsing and check if the right text is loaded
	char file_text[1024];
	char file_text_raw[1024];

	memset(file_text, 0, sizeof(file_text));
	memset(file_text_raw, 0, sizeof(file_text_raw));

	pause_parse();

	read_file_text("test2.tbl", CF_TYPE_TABLES, file_text, file_text_raw);
	reset_parse(file_text);

	required_string("#Begin");
	required_string("#End");

	unpause_parse();

	// We should be back in the original file
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

