
#include <gtest/gtest.h>

#include "osapi/dialogs.h"
#include "mod_table/mod_table.h"

#include "util/FSTestFixture.h"
#include "util/test_util.h"

class ModTableTest : public test::FSTestFixture {
 public:
	ModTableTest() : test::FSTestFixture(INIT_CFILE) {
		pushModDir("mod_table");
	}

 protected:
	virtual void SetUp() override {
		test::FSTestFixture::SetUp();

		// Make sure that we have a clear suppression list
		Suppressed_warning_categories.clear();

		mod_table_init();
	}
	virtual void TearDown() override {
		test::FSTestFixture::TearDown();
	}
};

TEST_F(ModTableTest, suppressed_warnings) {
	DEBUG_TEST();

	ASSERT_THROW(CategoryWarning(LOCATION, "NonSuppressedWarning", "TestWarning!"), os::dialogs::WarningException);

	ASSERT_NO_THROW(CategoryWarning(LOCATION, "SuppressedWarning", "TestWarning!"));
}

TEST_F(ModTableTest, future_targetted_version) {
	ASSERT_THROW(mod_table_init(), os::dialogs::ErrorException);
}

TEST_F(ModTableTest, correct_targetted_version) {
	mod_table_init();
}

TEST_F(ModTableTest, mod_support_test) {
	mod_table_init();

	ASSERT_TRUE(mod_supports_version(3, 7, 0));
	ASSERT_FALSE(mod_supports_version(4, 0, 0));
}
