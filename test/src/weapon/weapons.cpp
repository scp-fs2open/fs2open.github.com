
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

TEST_F(WeaponsParseTest, dinky_inheritance) {
	weapon_init();

	int idx = weapon_info_lookup("Test weapon");
	ASSERT_GE(idx, 0);

	const auto &wi = Weapon_info[idx];
	ASSERT_EQ(wi.dinky_shockwave_specified_fields, SCI_INNER_RAD);
	ASSERT_FLOAT_EQ(wi.dinky_shockwave.inner_rad, 5.0f);
	ASSERT_FLOAT_EQ(wi.dinky_shockwave.outer_rad, 40.0f);
	ASSERT_FLOAT_EQ(wi.dinky_shockwave.speed, 80.0f);
	ASSERT_FLOAT_EQ(wi.dinky_shockwave.damage, 200.0f);
	ASSERT_FALSE(wi.dinky_shockwave.damage_overridden);
}
