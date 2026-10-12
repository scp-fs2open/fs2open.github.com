
#include <gtest/gtest.h>

#include "util/FSTestFixture.h"
#include "util/test_util.h"

#include "utils/string_utils.h"
#include "localization/localize.h"
#include "weapon/weapon.h"

extern SCP_vector<SCP_string> Delayed_SSM_names;
extern SCP_vector<SCP_string> Delayed_SSM_indices;

static void expect_init_warning(const char* fragment)
{
	try {
		weapon_init();
		FAIL() << "weapon_init() did not warn";
	} catch (const os::dialogs::WarningException& e) {
		EXPECT_NE(SCP_string(e.what()).find(fragment), SCP_string::npos) << "warning was: " << e.what();
	}
}

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
		Weapon_info.clear();

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

TEST_F(WeaponsParseTest, template_applies_to_new_weapon) {
	weapon_init();

	EXPECT_LT(weapon_info_lookup("Base Laser"), 0);

	int derived = weapon_info_lookup("Derived Laser");
	ASSERT_GE(derived, 0);
	EXPECT_EQ(Weapon_info[derived].damage, 9.0f);
	EXPECT_EQ(Weapon_info[derived].max_speed, 500.0f);
	EXPECT_STREQ(Weapon_info[derived].title, "Base");
	EXPECT_EQ(Weapon_info[derived].subtype, WP_LASER);

	int plain = weapon_info_lookup("Plain Laser");
	ASSERT_GE(plain, 0);
	EXPECT_EQ(Weapon_info[plain].damage, 7.0f);
	EXPECT_STREQ(Weapon_info[plain].name, "Plain Laser");
}

TEST_F(WeaponsParseTest, template_of_template) {
	weapon_init();

	int leaf = weapon_info_lookup("Leaf");
	ASSERT_GE(leaf, 0);
	EXPECT_EQ(Weapon_info[leaf].max_speed, 321.0f);
	EXPECT_EQ(Weapon_info[leaf].damage, 3.0f);
	EXPECT_LT(weapon_info_lookup("Root"), 0);
	EXPECT_LT(weapon_info_lookup("Branch"), 0);
}

TEST_F(WeaponsParseTest, weapon_as_template) {
	weapon_init();

	int alpha = weapon_info_lookup("Alpha");
	int beta = weapon_info_lookup("Beta");
	ASSERT_GE(alpha, 0);
	ASSERT_GE(beta, 0);
	EXPECT_EQ(Weapon_info[beta].max_speed, 640.0f);
	EXPECT_EQ(Weapon_info[beta].damage, 2.0f);
	EXPECT_EQ(Weapon_info[alpha].damage, 4.0f);
	EXPECT_STREQ(Weapon_info[beta].name, "Beta");
}

TEST_F(WeaponsParseTest, secondary_from_untyped_template_detonates_on_expiration) {
	weapon_init();

	int derived = weapon_info_lookup("Derived Missile");
	ASSERT_GE(derived, 0);
	EXPECT_EQ(Weapon_info[derived].subtype, WP_MISSILE);
	EXPECT_TRUE(Weapon_info[derived].wi_flags[Weapon::Info_Flags::Detonate_on_expiration]);
}

TEST_F(WeaponsParseTest, template_display_name_rebuilt) {
	weapon_init();

	int derived = weapon_info_lookup("Derived#Laser");
	ASSERT_GE(derived, 0);
	EXPECT_TRUE(Weapon_info[derived].has_display_name());
	EXPECT_EQ(Weapon_info[derived].display_name, "Derived");
}

TEST_F(WeaponsParseTest, missing_template_warns) {
	DEBUG_TEST();

	expect_init_warning("Unable to find weapon template 'No Such Template'");
}

TEST_F(WeaponsParseTest, template_self_reference_warns) {
	DEBUG_TEST();

	expect_init_warning("requested itself as a template");
}

TEST_F(WeaponsParseTest, template_with_nocreate_warns) {
	DEBUG_TEST();

	expect_init_warning("+nocreate flag used on weapon template 'Base Laser'");
}

TEST_F(WeaponsParseTest, template_on_existing_weapon_warns) {
	DEBUG_TEST();

	for (int attempt = 0; attempt < 2; ++attempt) {
		expect_init_warning("A template was specified for weapon 'Alpha', which already exists");
		weapon_close();
	}
}

TEST_F(WeaponsParseTest, template_free_flight_time_kept) {
	weapon_init();

	int d = weapon_info_lookup("Seeker Derived");
	ASSERT_GE(d, 0);
	EXPECT_TRUE(Weapon_info[d].is_homing());
	EXPECT_EQ(Weapon_info[d].free_flight_time, 0.3f);
}

TEST_F(WeaponsParseTest, untyped_template_free_flight_warns) {
#ifndef NDEBUG
	expect_init_warning("sets no $Free Flight Time:; keeping 0.00 from the template");
#else
	weapon_init();

	int p = weapon_info_lookup("Seeker Primary");
	int s = weapon_info_lookup("Seeker Secondary");
	ASSERT_GE(p, 0);
	ASSERT_GE(s, 0);
	EXPECT_EQ(Weapon_info[p].free_flight_time, 0.0f);
	EXPECT_EQ(Weapon_info[s].free_flight_time, 0.0f);
#endif
}

TEST_F(WeaponsParseTest, template_laser_color_kept) {
	weapon_init();

	int d = weapon_info_lookup("Tinted Derived");
	ASSERT_GE(d, 0);
	EXPECT_EQ(Weapon_info[d].laser_color_2.red, 0);
	EXPECT_EQ(Weapon_info[d].laser_color_2.blue, 255);
}

TEST_F(WeaponsParseTest, template_ssm_copied) {
	weapon_init();

	auto listed = [](const char* n) {
		return std::find(Delayed_SSM_names.begin(), Delayed_SSM_names.end(), n) != Delayed_SSM_names.end();
	};
	EXPECT_TRUE(listed("Artillery Derived"));
	EXPECT_FALSE(listed("Artillery Base"));
	EXPECT_FALSE(listed("$Template:Artillery Base"));
	Delayed_SSM_names.clear();
}

TEST_F(WeaponsParseTest, template_proximity_lists_follow_name) {
	DEBUG_TEST();

	weapon_init();

	try {
		weapon_post_ship_init();
		FAIL() << "no warning: the proximity list was lost";
	} catch (const os::dialogs::AssertException& e) {
		FAIL() << "garbage index: " << e.what();
	} catch (const os::dialogs::WarningException& e) {
		EXPECT_NE(SCP_string(e.what()).find("Weapon 'Flak Derived': +Proximity Type entry 'fighter' not found"), SCP_string::npos) << e.what();
	}
}

TEST_F(WeaponsParseTest, template_same_name_as_weapon_keeps_ssm) {
	weapon_init();

	EXPECT_NE(std::find(Delayed_SSM_names.begin(), Delayed_SSM_names.end(), "Trebuchet"), Delayed_SSM_names.end());
#ifndef NDEBUG
	try {
		validate_SSM_entries();
		FAIL() << "validate_SSM_entries saw no entry for Trebuchet";
	} catch (const os::dialogs::WarningException& e) {
		EXPECT_NE(SCP_string(e.what()).find("SSM entry 'Trebuchet Strike' in specification for Trebuchet"), SCP_string::npos) << e.what();
	}
#endif
	Delayed_SSM_names.clear();
}

TEST_F(WeaponsParseTest, template_with_remove_warns) {
	DEBUG_TEST();

	expect_init_warning("+remove flag used on weapon template 'Doomed'");
}

TEST_F(WeaponsParseTest, template_display_name_inherited) {
	weapon_init();

	int d = weapon_info_lookup("Derived#Laser");
	ASSERT_GE(d, 0);
	EXPECT_EQ(Weapon_info[d].display_name, "Shown");
}

TEST_F(WeaponsParseTest, template_hashed_name_not_inherited) {
	weapon_init();

	int d = weapon_info_lookup("Subach#Mk2");
	ASSERT_GE(d, 0);
	EXPECT_EQ(Weapon_info[d].display_name, "Subach");
}

TEST_F(WeaponsParseTest, template_german_display_name_not_inherited) {
	Lcl_gr = 1;
	weapon_init();
	Lcl_gr = 0;

	int d = weapon_info_lookup("Derived Laser");
	ASSERT_GE(d, 0);
	EXPECT_TRUE(Weapon_info[d].has_display_name());
	EXPECT_EQ(Weapon_info[d].display_name, "Derived Laser");
}

TEST_F(WeaponsParseTest, weapon_as_template_self_warns) {
	DEBUG_TEST();

	expect_init_warning("Weapon 'Foo' requested itself as a template");
}

TEST_F(WeaponsParseTest, template_with_weapon_as_template_warns) {
#ifndef NDEBUG
	expect_init_warning("'+Use Weapon as Template:' is not supported on weapon template 'Base Laser'");
#else
	weapon_init();

	EXPECT_GE(weapon_info_lookup("Survivor"), 0);
#endif
}

TEST_F(WeaponsParseTest, untyped_template_keeps_ballistic) {
	weapon_init();

	int d = weapon_info_lookup("Gun Derived");
	ASSERT_GE(d, 0);
	EXPECT_TRUE(Weapon_info[d].wi_flags[Weapon::Info_Flags::Ballistic]);
	EXPECT_EQ(Weapon_info[d].rearm_rate, 1.0f / 50.0f);
}

TEST_F(WeaponsParseTest, untyped_template_ballistic_rearm_warns) {
	DEBUG_TEST();

	expect_init_warning("Ballistic primary Gun Derived was built from a template with no $Subtype: and sets no $Rearm Rate; keeping 1.00");
}

TEST_F(WeaponsParseTest, template_substitution_follows_name) {
	weapon_init();

	int d = weapon_info_lookup("Sub Derived");
	int other = weapon_info_lookup("Sub Other");
	ASSERT_GE(d, 0);
	ASSERT_GE(other, 0);
	ASSERT_EQ(Weapon_info[d].num_substitution_patterns, 2u);
	EXPECT_EQ(Weapon_info[d].weapon_substitution_pattern[0], d);
	EXPECT_EQ(Weapon_info[d].weapon_substitution_pattern[1], other);
}

TEST_F(WeaponsParseTest, template_ssm_numeric_override) {
	weapon_init();

	auto listed = [](const SCP_vector<SCP_string>& v, const char* n) { return std::find(v.begin(), v.end(), n) != v.end(); };
	EXPECT_FALSE(listed(Delayed_SSM_names, "Artillery Numeric"));
	EXPECT_TRUE(listed(Delayed_SSM_indices, "Artillery Numeric"));
	Delayed_SSM_names.clear();
	Delayed_SSM_indices.clear();
}

TEST_F(WeaponsParseTest, both_template_directives_warn) {
	DEBUG_TEST();

	expect_init_warning("ignoring '+Use Weapon as Template:'");
}

TEST_F(WeaponsParseTest, duplicate_template_warns) {
	DEBUG_TEST();

	expect_init_warning("Weapon template Twin already exists");
}

TEST_F(WeaponsParseTest, nocreate_with_template_warns) {
	DEBUG_TEST();

	expect_init_warning("Both '+nocreate' and a template were specified for weapon 'Ghost'");
}

TEST_F(WeaponsParseTest, missing_weapon_as_template_warns) {
	DEBUG_TEST();

	expect_init_warning("Unable to find weapon 'Nope' requested as a template by weapon 'Orphan'");
}

TEST_F(WeaponsParseTest, untyped_template_reloaded_per_batch_by_section) {
	weapon_init();

	int p = weapon_info_lookup("Any Primary");
	int s = weapon_info_lookup("Any Secondary");
	ASSERT_GE(p, 0);
	ASSERT_GE(s, 0);
	EXPECT_EQ(Weapon_info[p].reloaded_per_batch, 100);
	EXPECT_EQ(Weapon_info[s].reloaded_per_batch, 4);
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
