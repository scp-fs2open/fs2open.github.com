#include <gtest/gtest.h>
#include <bmpman/bmpman.h>
#include <cfile/cfile.h>
#include <globalincs/systemvars.h>
#include <graphics/texture_material.h>
#include <lighting/lighting_profiles.h>
#include <osapi/dialogs.h>
#include <parse/parselo.h>

#include <cmath>

#include "util/FSTestFixture.h"

namespace {

// Debug test builds turn Warning() into an exception, Release builds only count it
void expect_warning(const char* name, int dir_type, const char* text)
{
#ifndef NDEBUG
	try {
		texture_material_find(name, dir_type);
		FAIL() << "expected a warning containing '" << text << "'";
	} catch (const os::dialogs::WarningException& e) {
		EXPECT_NE(SCP_string(e.what()).find(text), SCP_string::npos) << e.what();
	}
#else
	int warnings = Global_warning_count;
	texture_material_find(name, dir_type);
	EXPECT_EQ(Global_warning_count, warnings + 1) << "expected a warning containing '" << text << "'";
#endif
}

} // namespace

class TextureMaterialTest : public test::FSTestFixture {
public:
	TextureMaterialTest() : test::FSTestFixture(INIT_CFILE) {
		pushModDir("graphics");
		pushModDir("texture_material");
	}

protected:
	void TearDown() override {
		texture_material_reset();
		test::FSTestFixture::TearDown();
	}
};

class TextureMaterialBitmapTest : public test::FSTestFixture {
public:
	TextureMaterialBitmapTest() : test::FSTestFixture(INIT_CFILE | INIT_GRAPHICS) {
		pushModDir("graphics");
		pushModDir("texture_material");
	}

protected:
	// the fixture runs as a standalone server, where bmpman swaps every name for a placeholder
	void SetUp() override {
		test::FSTestFixture::SetUp();
		_was_standalone = Is_standalone;
		Is_standalone = 0;
	}

	void TearDown() override {
		Is_standalone = _was_standalone;
		test::FSTestFixture::TearDown();
		texture_material_reset();
	}

private:
	int _was_standalone = 0;
};

TEST_F(TextureMaterialTest, reads_both_glow_values) {
	auto mat = texture_material_find("mat_glow", CF_TYPE_MAPS);
	ASSERT_NE(mat, nullptr);
	EXPECT_FLOAT_EQ(*mat->glow.intensity, 2.5f);
	EXPECT_FLOAT_EQ(*mat->glow.hdr_multiplier, 6.0f);
}

TEST_F(TextureMaterialTest, unset_values_stay_unset_and_comments_are_skipped) {
	auto mat = texture_material_find("mat_intensity_only", CF_TYPE_MAPS);
	ASSERT_NE(mat, nullptr);
	EXPECT_FLOAT_EQ(*mat->glow.intensity, 0.75f);
	EXPECT_FALSE(mat->glow.hdr_multiplier.has_value());

	auto inline_comment = texture_material_find("mat_inline_comment", CF_TYPE_MAPS);
	ASSERT_NE(inline_comment, nullptr);
	EXPECT_FLOAT_EQ(*inline_comment->glow.intensity, 1.5f);
}

TEST_F(TextureMaterialTest, crlf_and_utf8_bom_are_read) {
	auto crlf = texture_material_find("mat_crlf", CF_TYPE_MAPS);
	ASSERT_NE(crlf, nullptr);
	EXPECT_FLOAT_EQ(*crlf->glow.intensity, 0.5f);
	EXPECT_FLOAT_EQ(*crlf->glow.hdr_multiplier, 2.25f);

	auto bom = texture_material_find("mat_bom", CF_TYPE_MAPS);
	ASSERT_NE(bom, nullptr);
	EXPECT_FLOAT_EQ(*bom->glow.intensity, 1.75f);
}

TEST_F(TextureMaterialTest, texture_without_a_mat_file) {
	EXPECT_EQ(texture_material_find("mat_plain", CF_TYPE_MAPS), nullptr);
	EXPECT_EQ(texture_material_find("", CF_TYPE_MAPS), nullptr);
	EXPECT_EQ(texture_material_find(nullptr, CF_TYPE_MAPS), nullptr);
}

TEST_F(TextureMaterialTest, extension_and_case_are_ignored) {
	auto upper = texture_material_find("MAT_GLOW.png", CF_TYPE_MAPS);
	ASSERT_NE(upper, nullptr);
	EXPECT_EQ(texture_material_find("mat_glow", CF_TYPE_MAPS), upper);
	EXPECT_EQ(texture_material_find("mat_glow.dds", CF_TYPE_MAPS), upper);
}

TEST_F(TextureMaterialTest, directory_type_is_part_of_the_lookup) {
	auto any = texture_material_find("mat_fx", CF_TYPE_ANY);
	ASSERT_NE(any, nullptr);
	EXPECT_FLOAT_EQ(*any->glow.intensity, 6.0f);
	EXPECT_EQ(texture_material_find("mat_fx", CF_TYPE_MAPS), nullptr);
	EXPECT_NE(texture_material_find("mat_fx", CF_TYPE_EFFECTS), nullptr);
}

TEST_F(TextureMaterialTest, non_text_files_are_rejected) {
	expect_warning("mat_sharp_s", CF_TYPE_MAPS, "not a plain-text");
	expect_warning("mat_utf16", CF_TYPE_MAPS, "not a plain-text");
	expect_warning("mat_encrypted", CF_TYPE_MAPS, "not a plain-text");
	expect_warning("mat_empty", CF_TYPE_MAPS, "is empty");
#ifdef NDEBUG
	EXPECT_EQ(texture_material_find("mat_sharp_s", CF_TYPE_MAPS), nullptr);
	EXPECT_EQ(texture_material_find("mat_empty", CF_TYPE_MAPS), nullptr);
#endif
}

TEST_F(TextureMaterialTest, a_rejected_file_warns_once) {
	expect_warning("mat_sharp_s", CF_TYPE_MAPS, "not a plain-text");
#ifdef NDEBUG
	int warnings = Global_warning_count;
	EXPECT_EQ(texture_material_find("mat_sharp_s", CF_TYPE_MAPS), nullptr);
	EXPECT_EQ(Global_warning_count, warnings);
#endif
}

TEST_F(TextureMaterialTest, bad_values_warn_and_are_ignored) {
	expect_warning("mat_bad_number", CF_TYPE_MAPS, "needs a number");
	expect_warning("mat_nan", CF_TYPE_MAPS, "needs a number");
	expect_warning("mat_no_value", CF_TYPE_MAPS, "needs a number");
#ifdef NDEBUG
	EXPECT_FALSE(texture_material_find("mat_bad_number", CF_TYPE_MAPS)->glow.intensity.has_value());
	EXPECT_FALSE(texture_material_find("mat_nan", CF_TYPE_MAPS)->glow.intensity.has_value());
	auto no_value = texture_material_find("mat_no_value", CF_TYPE_MAPS);
	EXPECT_FALSE(no_value->glow.intensity.has_value());
	EXPECT_FLOAT_EQ(*no_value->glow.hdr_multiplier, 4.0f);
#endif
}

TEST_F(TextureMaterialTest, out_of_range_values_are_clamped) {
	expect_warning("mat_negative", CF_TYPE_MAPS, "negative");
	expect_warning("mat_huge", CF_TYPE_MAPS, "above");
#ifdef NDEBUG
	EXPECT_FLOAT_EQ(*texture_material_find("mat_negative", CF_TYPE_MAPS)->glow.intensity, 0.0f);
	EXPECT_FLOAT_EQ(*texture_material_find("mat_huge", CF_TYPE_MAPS)->glow.intensity,
		lighting_profiles::MAX_GLOW_MAP_VALUE);
#endif
}

TEST_F(TextureMaterialTest, extra_text_and_unknown_entries_warn_and_keep_the_rest) {
	expect_warning("mat_trailing", CF_TYPE_MAPS, "extra text");
	expect_warning("mat_unknown_key", CF_TYPE_MAPS, "unrecognized");
#ifdef NDEBUG
	EXPECT_FLOAT_EQ(*texture_material_find("mat_trailing", CF_TYPE_MAPS)->glow.intensity, 2.0f);
	EXPECT_FLOAT_EQ(*texture_material_find("mat_unknown_key", CF_TYPE_MAPS)->glow.intensity, 1.25f);
#endif
}

TEST_F(TextureMaterialTest, the_largest_values_fit_the_emissive_buffer) {
	// the emissive render target is 16-bit float, whose largest finite value is 65504
	texture_glow_settings glow;
	glow.intensity = lighting_profiles::MAX_GLOW_MAP_VALUE;
	EXPECT_LE(texture_glow_intensity(glow) * texture_glow_hdr_multiplier(glow), 65504.0f);
}

TEST_F(TextureMaterialTest, many_unknown_entries_warn_once_and_are_counted) {
#ifndef NDEBUG
	EXPECT_THROW(texture_material_find("mat_many_unknown", CF_TYPE_MAPS), os::dialogs::WarningException);
#else
	// one warning for the first unknown entry, one summary for the other three
	int warnings = Global_warning_count;
	auto mat = texture_material_find("mat_many_unknown", CF_TYPE_MAPS);
	EXPECT_EQ(Global_warning_count, warnings + 2);
	ASSERT_NE(mat, nullptr);
	EXPECT_FLOAT_EQ(*mat->glow.intensity, 1.25f);
#endif
}

TEST_F(TextureMaterialTest, an_outer_parse_resumes_after_a_mat_file) {
#ifndef NDEBUG
	EXPECT_THROW(texture_material_find("mat_bad_number", CF_TYPE_MAPS), os::dialogs::WarningException);
	// a parse left paused makes this read fail
	EXPECT_NO_THROW(read_file_text("mat_glow.mat", CF_TYPE_MAPS));
#endif
	char outer[] = "$Outer: 1\n";
	reset_parse(outer);
	pause_parse();
	const texture_material* mat = nullptr;
	try {
		mat = texture_material_find("mat_intensity_only", CF_TYPE_MAPS);
	} catch (...) {
		unpause_parse();
		throw;
	}
	unpause_parse();
	EXPECT_NE(mat, nullptr);
	EXPECT_EQ(Mp, outer);
}

TEST_F(TextureMaterialBitmapTest, loaded_texture_carries_its_material) {
	int textured = bm_load("mat_textured", CF_TYPE_MAPS);
	ASSERT_GE(textured, 0);
	auto mat = bm_get_texture_material(textured);
	ASSERT_NE(mat, nullptr);
	EXPECT_FLOAT_EQ(*mat->glow.intensity, 4.0f);

	int plain = bm_load("mat_plain", CF_TYPE_MAPS);
	ASSERT_GE(plain, 0);
	EXPECT_EQ(bm_get_texture_material(plain), nullptr);

	EXPECT_EQ(bm_get_texture_material(-1), nullptr);
}

TEST_F(TextureMaterialBitmapTest, eff_frames_share_the_material) {
	int nframes = 0;
	int first = bm_load_animation("mat_anim", &nframes, nullptr, nullptr, nullptr, false, CF_TYPE_MAPS);
	ASSERT_GE(first, 0);
	ASSERT_EQ(nframes, 2);

	for (int i = 0; i < nframes; i++) {
		auto mat = bm_get_texture_material(first + i);
		ASSERT_NE(mat, nullptr);
		EXPECT_FLOAT_EQ(*mat->glow.hdr_multiplier, 8.0f);
	}
}

TEST_F(TextureMaterialBitmapTest, apng_frames_share_the_material) {
	int nframes = 0;
	int first = bm_load_animation("mat_apng", &nframes, nullptr, nullptr, nullptr, false, CF_TYPE_MAPS);
	ASSERT_GE(first, 0);
	ASSERT_EQ(nframes, 2);

	for (int i = 0; i < nframes; i++) {
		auto mat = bm_get_texture_material(first + i);
		ASSERT_NE(mat, nullptr);
		EXPECT_FLOAT_EQ(*mat->glow.intensity, 9.0f);
	}
}
