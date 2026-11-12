#include <gtest/gtest.h>
#include <graphics/material.h>
#include <lighting/lighting_profiles.h>

#include "util/FSTestFixture.h"

static float profile_hdr_multiplier()
{
	return lighting_profiles::current_glow_map_hdr_multiplier(lighting_profiles::current_glow_map_intensity());
}

class GlowSettingsTest : public test::FSTestFixture {
public:
	GlowSettingsTest() : test::FSTestFixture(INIT_CFILE) {
		pushModDir("lighting");
	}

protected:
	void SetUp() override {
		test::FSTestFixture::SetUp();
		lighting_profiles::load_profiles();
	}

	void TearDown() override {
		lighting_profiles::switch_to(lighting_profiles::default_name());
		test::FSTestFixture::TearDown();
	}
};

TEST_F(GlowSettingsTest, defaults_match_the_old_shader_constants) {
	lighting_profiles::switch_to("Glow Default");
	ASSERT_EQ(lighting_profiles::current()->name, "Glow Default");
	EXPECT_FLOAT_EQ(lighting_profiles::current_glow_map_intensity(), 1.5f);
	EXPECT_FLOAT_EQ(profile_hdr_multiplier(), 3.0f);
}

TEST_F(GlowSettingsTest, hdr_multiplier_follows_intensity_unless_set) {
	lighting_profiles::switch_to("Glow Intensity");
	EXPECT_FLOAT_EQ(lighting_profiles::current_glow_map_intensity(), 2.0f);
	EXPECT_FLOAT_EQ(profile_hdr_multiplier(), 4.0f);

	lighting_profiles::switch_to("Glow Both");
	EXPECT_FLOAT_EQ(lighting_profiles::current_glow_map_intensity(), 2.0f);
	EXPECT_FLOAT_EQ(profile_hdr_multiplier(), 5.0f);

	lighting_profiles::switch_to("Glow Intensity");
	EXPECT_FLOAT_EQ(profile_hdr_multiplier(), 4.0f);
}

TEST_F(GlowSettingsTest, texture_values_override_the_profile) {
	lighting_profiles::switch_to("Glow Both");
	model_material material;
	EXPECT_FLOAT_EQ(material.get_glow_intensity(), 2.0f);
	EXPECT_FLOAT_EQ(material.get_glow_hdr_multiplier(), 5.0f);

	material.set_glow({3.0f, std::nullopt});
	EXPECT_FLOAT_EQ(material.get_glow_intensity(), 3.0f);
	EXPECT_FLOAT_EQ(material.get_glow_hdr_multiplier(), 5.0f);

	material.set_glow({std::nullopt, 7.0f});
	EXPECT_FLOAT_EQ(material.get_glow_intensity(), 2.0f);
	EXPECT_FLOAT_EQ(material.get_glow_hdr_multiplier(), 7.0f);

	material.set_glow({});
	EXPECT_FLOAT_EQ(material.get_glow_intensity(), 2.0f);
	EXPECT_FLOAT_EQ(material.get_glow_hdr_multiplier(), 5.0f);

	lighting_profiles::switch_to("Glow Intensity");
	material.set_glow({3.0f, std::nullopt});
	EXPECT_FLOAT_EQ(material.get_glow_hdr_multiplier(), 6.0f);
}

class GlowNegativeProfileTest : public test::FSTestFixture {
public:
	GlowNegativeProfileTest() : test::FSTestFixture(INIT_CFILE) {
		pushModDir("lighting");
	}

protected:
	void TearDown() override {
#ifdef NDEBUG
		lighting_profiles::switch_to(lighting_profiles::default_name());
#endif
		test::FSTestFixture::TearDown();
	}
};

TEST_F(GlowNegativeProfileTest, negative_glow_values_clamp_to_zero) {
#ifndef NDEBUG
	GTEST_SKIP() << "in debug test builds Warning() throws mid-parse";
#else
	int warnings = Global_warning_count;
	lighting_profiles::load_profiles();
	EXPECT_EQ(Global_warning_count, warnings + 2);

	lighting_profiles::switch_to("Glow Negative");
	EXPECT_FLOAT_EQ(lighting_profiles::current_glow_map_intensity(), 0.0f);
	EXPECT_FLOAT_EQ(profile_hdr_multiplier(), 0.0f);

	lighting_profiles::switch_to("Glow Negative HDR");
	EXPECT_FLOAT_EQ(lighting_profiles::current_glow_map_intensity(), 1.5f);
	EXPECT_FLOAT_EQ(profile_hdr_multiplier(), 0.0f);
#endif
}

TEST_F(GlowNegativeProfileTest, bad_glow_values_are_rejected) {
#ifndef NDEBUG
	GTEST_SKIP() << "in debug test builds Warning() throws mid-parse";
#else
	int warnings = Global_warning_count;
	lighting_profiles::load_profiles();
	EXPECT_EQ(Global_warning_count, warnings + 4);

	lighting_profiles::switch_to("Glow NaN");
	EXPECT_FLOAT_EQ(lighting_profiles::current_glow_map_intensity(), 1.5f);
	EXPECT_FLOAT_EQ(profile_hdr_multiplier(), 3.0f);

	lighting_profiles::switch_to("Glow Huge");
	EXPECT_FLOAT_EQ(lighting_profiles::current_glow_map_intensity(), lighting_profiles::MAX_GLOW_MAP_VALUE);
	EXPECT_FLOAT_EQ(profile_hdr_multiplier(), lighting_profiles::MAX_GLOW_MAP_VALUE);
#endif
}
