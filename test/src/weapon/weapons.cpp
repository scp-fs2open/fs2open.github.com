
#include <gtest/gtest.h>

#include "util/FSTestFixture.h"

#include "weapon/weapon.h"

class WeaponsParseTest : public test::FSTestFixture {
 public:
	WeaponsParseTest() : test::FSTestFixture(INIT_CFILE) {
		pushModDir("weapon");
	}

 protected:
	virtual void SetUp() override {
		test::FSTestFixture::SetUp();
	}
	virtual void TearDown() override {
		weapon_close();

		test::FSTestFixture::TearDown();
	}
};

TEST_F(WeaponsParseTest, description_line_too_long) {
	ASSERT_THROW(weapon_init(), os::dialogs::WarningException);
}

TEST_F(WeaponsParseTest, description_too_many_lines) {
	ASSERT_THROW(weapon_init(), os::dialogs::WarningException);
}
