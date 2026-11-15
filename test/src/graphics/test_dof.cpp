#include <gtest/gtest.h>

#include <string>

#include "def_files/def_files.h"
#include "graphics/2d.h"
#include "graphics/dof.h"
#include "options/Option.h"
#include "options/OptionsManager.h"
#include "render/3d.h"

using namespace graphics;

namespace {
// The OptionsManager is one object for the whole process, so remove the requirements after each test.
class Dof : public ::testing::Test {
  protected:
	void TearDown() override { options::OptionsManager::instance()->clearRequirements(); }
};

const char* const KEYS[] = {"Graphics.DepthOfField", "Graphics.DepthOfFieldMaxBlur", "Graphics.DepthOfFieldDistance"};
} // namespace

TEST_F(Dof, FarObjectsStaySharp)
{
	EXPECT_EQ(dof_coc(50.0f, 50.0f, 8.0f), 0.0f);
	EXPECT_EQ(dof_coc(5000.0f, 50.0f, 8.0f), 0.0f);
}

TEST_F(Dof, BlurGrowsAsTheObjectComesNearer)
{
	// thin lens with the focus at infinity: K * (1/z - 1/N), K = max_blur * N
	EXPECT_FLOAT_EQ(dof_coc(40.0f, 50.0f, 8.0f), 8.0f * (50.0f / 40.0f - 1.0f));
	EXPECT_LT(dof_coc(40.0f, 50.0f, 8.0f), dof_coc(30.0f, 50.0f, 8.0f));
}

TEST_F(Dof, BlurIsLargestFromHalfTheDistance)
{
	EXPECT_FLOAT_EQ(dof_coc(25.0f, 50.0f, 8.0f), 8.0f);
	EXPECT_FLOAT_EQ(dof_coc(1.0f, 50.0f, 8.0f), 8.0f);
	EXPECT_FLOAT_EQ(dof_coc(0.0f, 50.0f, 8.0f), 8.0f);
}

TEST_F(Dof, BlurScalesWithResolutionAndZoom)
{
	// the default view: zoom 0.75 times PROJ_FOV_FACTOR, as g3_set_fov() does
	const float default_fov = 0.75f * PROJ_FOV_FACTOR;
	EXPECT_FLOAT_EQ(dof_pixel_scale(1080.0f, default_fov), 1.0f);
	EXPECT_FLOAT_EQ(dof_pixel_scale(2160.0f, default_fov), 2.0f);
	// a narrower field of view (zoom in) is a longer focal length, so more blur
	EXPECT_FLOAT_EQ(dof_pixel_scale(1080.0f, 0.5f * PROJ_FOV_FACTOR),
		tanf(0.375f * PROJ_FOV_FACTOR) / tanf(0.25f * PROJ_FOV_FACTOR));
}

TEST_F(Dof, MoreTapsForAWiderBlur)
{
	EXPECT_EQ(dof_samples(8.0f), 96);
	EXPECT_EQ(dof_samples(16.0f), 96);
	EXPECT_EQ(dof_samples(24.0f), 216);
	EXPECT_EQ(dof_samples(64.0f), DOF_MAX_SAMPLES);
}

TEST_F(Dof, ShaderTapLimitMatchesTheHeader)
{
	// dof_samples() never asks for more taps than the shader's loop allows.
	const auto shader = defaults_get_file("dof-f.sdr");
	ASSERT_NE(shader.data, nullptr);
	const SCP_string text(static_cast<const char*>(shader.data), shader.size);
	EXPECT_NE(text.find("const int MAX_SAMPLES = " + std::to_string(DOF_MAX_SAMPLES) + ";"), SCP_string::npos);
}

TEST_F(Dof, ShaderTileSizeMatchesTheHeader)
{
	const auto shader = defaults_get_file("dof-common.sdr");
	ASSERT_NE(shader.data, nullptr);
	const SCP_string text(static_cast<const char*>(shader.data), shader.size);
	EXPECT_NE(text.find("const int DOF_TILE = " + std::to_string(DOF_TILE_SIZE) + ";"), SCP_string::npos);
}

TEST_F(Dof, OptionsExistAndDefaultOff)
{
	auto* mgr = options::OptionsManager::instance();
	for (const char* key : KEYS) {
		EXPECT_NE(mgr->getOptionByKey(key), nullptr) << key;
	}
	// Off by default, so the default render does not change.
	dof_frame frame;
	EXPECT_FALSE(dof_get_frame(&frame));
}

TEST_F(Dof, FrameFollowsTheOptions)
{
	auto* mgr = options::OptionsManager::instance();
	auto set = [mgr](const char* key, const char* value) {
		mgr->setOverride(key, value, "test");
		mgr->getOptionByKey(key)->loadInitial();
		mgr->clearOverrides();
	};
	const int saved_h = gr_screen.max_h;
	const fov_t saved_fov = Proj_fov;
	gr_screen.max_h = 2160;
	Proj_fov = 0.75f * PROJ_FOV_FACTOR;
	set("Graphics.DepthOfField", "true");
	set("Graphics.DepthOfFieldMaxBlur", "12");
	set("Graphics.DepthOfFieldDistance", "80");

	dof_frame frame;
	ASSERT_TRUE(dof_get_frame(&frame));
	EXPECT_FLOAT_EQ(frame.near_dist, 80.0f);
	EXPECT_FLOAT_EQ(frame.max_blur, 24.0f); // 12 px at 1080 lines is 24 px at 2160
	EXPECT_EQ(frame.samples, dof_samples(frame.max_blur));

	// the options bind once, so put the defaults back for any later test
	set("Graphics.DepthOfField", "false");
	set("Graphics.DepthOfFieldMaxBlur", "16");
	set("Graphics.DepthOfFieldDistance", "50");
	gr_screen.max_h = saved_h;
	Proj_fov = saved_fov;
	EXPECT_FALSE(dof_get_frame(&frame));
}

TEST_F(Dof, EveryOptionRequiresOpenGL)
{
	auto* mgr = options::OptionsManager::instance();
	dof_set_requirements();
	for (const char* key : KEYS) {
		auto requirement = mgr->getRequirement(key);
		ASSERT_TRUE(requirement.has_value()) << key;
		EXPECT_EQ(requirement->first, "opengl") << key;
		EXPECT_FALSE(requirement->second.empty()) << key;
	}
	EXPECT_FALSE(mgr->getRequirement("Graphics.VSync").has_value());
}
