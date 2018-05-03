
#include <gtest/gtest.h>

#include "util/FSTestFixture.h"
#include "util/test_util.h"

#include "weapon/weapon.h"

class WeaponsParseTest : public test::FSTestFixture {
 public:
	WeaponsParseTest() : test::FSTestFixture(INIT_CFILE) {
		pushModDir("weapon");
	}

 protected:
	void SetUp() override {
		test::FSTestFixture::SetUp();
	}
	void TearDown() override {
		weapon_close();

		test::FSTestFixture::TearDown();
	}
};

TEST_F(WeaponsParseTest, description_line_too_long) {
	DEBUG_TEST();

	ASSERT_THROW(weapon_init(), os::dialogs::WarningException);
}

TEST_F(WeaponsParseTest, description_too_many_lines) {
	DEBUG_TEST();

	ASSERT_THROW(weapon_init(), os::dialogs::WarningException);
}
