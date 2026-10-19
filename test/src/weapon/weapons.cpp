
#include <gtest/gtest.h>

#include "util/FSTestFixture.h"
#include "util/test_util.h"

#include "utils/string_utils.h"
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

TEST(WeaponInfoCloneTest, copies_members) {
	weapon_info src;
	strcpy_s(src.name, "Template Laser");
	src.display_name = "Shown Name";
	src.desc = util::unique_copy("a description", true);
	src.failure_sub_name = util::unique_copy("Sub Weapon", true);
	src.detail_distance[2] = 1234;
	strcpy_s(src.weapon_substitution_pattern_names[1], "Other Laser");
	src.spawn_info[1].spawn_count = 7;
	src.damage = 42.5f;
	src.wi_flags.set(Weapon::Info_Flags::Homing_heat);
	src.proximity_iff = {1, 3};
	src.b_info.beam_iff_miss_factor[2] = {{0.1f, 0.2f, 0.3f, 0.4f, 0.5f}};
	src.b_info.t5info.burst_rot_pattern = {0.25f};
	src.custom_data["key"] = "value";

	weapon_info dest;
	dest.clone(src);

	EXPECT_STREQ(dest.name, "Template Laser");
	EXPECT_EQ(dest.display_name, "Shown Name");
	ASSERT_NE(dest.desc, nullptr);
	EXPECT_STREQ(dest.desc.get(), "a description");
	ASSERT_NE(dest.failure_sub_name, nullptr);
	EXPECT_STREQ(dest.failure_sub_name.get(), "Sub Weapon");
	EXPECT_EQ(dest.detail_distance[2], 1234);
	EXPECT_STREQ(dest.weapon_substitution_pattern_names[1], "Other Laser");
	EXPECT_EQ(dest.spawn_info[1].spawn_count, 7);
	EXPECT_EQ(dest.damage, 42.5f);
	EXPECT_TRUE(dest.wi_flags[Weapon::Info_Flags::Homing_heat]);
	EXPECT_EQ(dest.proximity_iff, src.proximity_iff);
	EXPECT_EQ(dest.b_info.beam_iff_miss_factor, src.b_info.beam_iff_miss_factor);
	EXPECT_EQ(dest.b_info.t5info.burst_rot_pattern, src.b_info.t5info.burst_rot_pattern);
	EXPECT_EQ(dest.custom_data, src.custom_data);
}
