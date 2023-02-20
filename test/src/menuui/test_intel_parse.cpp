#include <gtest/gtest.h>

#include "menuui/techmenu.h"
#include "localization/localize.h"

#include "util/FSTestFixture.h"

class IntelParseTest : public test::FSTestFixture {
 public:
	IntelParseTest() : test::FSTestFixture(INIT_CFILE) {
		pushModDir("menuui");
		pushModDir("intel_parse");
	}

 protected:
	void SetUp() override {
		test::FSTestFixture::SetUp();

		// techroom_intel_init() is not here so it is handled in
		// the test if it throws.
	}
	void TearDown() override {
		// this is here so that it is run even when a test fails
		techroom_intel_reset();

		test::FSTestFixture::TearDown();
	}
};

// Commonly used expected intel data entries.
static intel_data expected_foo = {
	"Foo name", // XSTR id 3000
	"Foo desc", // XSTR id 3001
	"Foo anim",
	IIF_IN_TECH_DATABASE | IIF_DEFAULT_IN_TECH_DATABASE
};
static intel_data expected_bar = {
	"Bar name", // XSTR id 3002
	"Bar desc", // XSTR id 3003
	"Bar anim",
	0
};
static intel_data expected_baz = {
	"Baz name", // XSTR id 3004
	"Baz desc", // XSTR id 3005
	"Baz anim",
	IIF_IN_TECH_DATABASE | IIF_DEFAULT_IN_TECH_DATABASE
};

static intel_data expected_qux = {
	"Qux name", // No XSTR
	"Qux desc", // No XSTR
	"Qux anim",
	IIF_IN_TECH_DATABASE | IIF_DEFAULT_IN_TECH_DATABASE
};

// use SCOPED_TRACE("<test name>") before calling this
static void test_intel_data_equal(const intel_data& i1, const intel_data& i2)
{
	EXPECT_STREQ(i1.name, i2.name);
	EXPECT_STREQ(i1.desc.c_str(), i2.desc.c_str());
	EXPECT_STREQ(i1.anim_filename, i2.anim_filename);
	EXPECT_EQ(i1.flags, i2.flags);
}

/*
 * These tests assume that the raw file is preprocessed correctly,
 * i.e. comments are removed correctly, necessary hacking on retail
 * data is done, ...
 * Also, XSTR("...", 123) parsing is assumed to be done correctly.
 * Only parsing of the actual contents is tested.
 */

// A missing file just means there are no entries.
TEST_F(IntelParseTest, missing_file) {
	techroom_intel_init();

	EXPECT_EQ(intel_info_size(), 0);
}

// The same with an empty file.
TEST_F(IntelParseTest, empty_file) {
	techroom_intel_init();

	EXPECT_EQ(intel_info_size(), 0);
}

// The same with only white space.
TEST_F(IntelParseTest, only_white_space) {
	techroom_intel_init();

	EXPECT_EQ(intel_info_size(), 0);
}

// A single valid entry.
TEST_F(IntelParseTest, single) {
	SCOPED_TRACE("single");

	techroom_intel_init();

	ASSERT_EQ(intel_info_size(), 1);

	test_intel_data_equal(expected_foo, Intel_info[0]);
}

// The same, but with translation to German.
TEST_F(IntelParseTest, single_translate) {
	SCOPED_TRACE("single_translate");

	lcl_xstr_close();
	lcl_set_language(LCL_GERMAN);
	lcl_xstr_init();

	techroom_intel_init();

	intel_data expected = {
		"Foo name German", // XSTR id 3000
		"Foo desc German", // XSTR id 3001
		"Foo anim",
		IIF_IN_TECH_DATABASE | IIF_DEFAULT_IN_TECH_DATABASE
	};

	ASSERT_EQ(intel_info_size(), 1);

	test_intel_data_equal(expected, Intel_info[0]);
}

// Rubbish followed by a valid entry.
// Parsing stops at the rubbish.
TEST_F(IntelParseTest, invalid_start) {
	techroom_intel_init();

	EXPECT_EQ(intel_info_size(), 0);
}

// Three valid entries.
TEST_F(IntelParseTest, three) {
	SCOPED_TRACE("three");

	techroom_intel_init();

	ASSERT_EQ(intel_info_size(), 3);

	test_intel_data_equal(expected_foo, Intel_info[0]);
	test_intel_data_equal(expected_bar, Intel_info[1]);
	test_intel_data_equal(expected_baz, Intel_info[2]);
}

// A valid entry followed by one with a missing $Name: line.
// The second entry should be ignored. Parsing should stop there.
TEST_F(IntelParseTest, missing_name) {
	SCOPED_TRACE("missing_name");

	EXPECT_ANY_THROW(techroom_intel_init());

	ASSERT_EQ(intel_info_size(), 1);

	test_intel_data_equal(expected_foo, Intel_info[0]);
}