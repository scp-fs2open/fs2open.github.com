#include "graphics/gtao.h"

#include "graphics/2d.h"
#include "options/Option.h"
#include "options/OptionsManager.h"

#include <gtest/gtest.h>

using namespace options;

namespace {
// The OptionsManager is one object for the whole process, so remove the requirements after each test.
class Gtao : public ::testing::Test {
  protected:
	void TearDown() override { OptionsManager::instance()->clearRequirements(); }
};

template <typename T>
const Option<T>* typed_option(const char* key)
{
	return static_cast<const Option<T>*>(OptionsManager::instance()->getOptionByKey(key));
}
} // namespace

TEST_F(Gtao, QualityLevelsSetSliceAndStepCounts)
{
	EXPECT_EQ(gtao_samples(0).slices, 0);
	EXPECT_EQ(gtao_samples(1).slices, 2);
	EXPECT_EQ(gtao_samples(1).steps, 4);
	// Medium is the 3 x 6 that the spike measured
	EXPECT_EQ(gtao_samples(2).slices, 3);
	EXPECT_EQ(gtao_samples(2).steps, 6);
	EXPECT_EQ(gtao_samples(3).slices, 4);
	EXPECT_EQ(gtao_samples(3).steps, 8);
	// A level from a hand-edited config file must not give a loop without an end
	EXPECT_EQ(gtao_samples(99).slices, 4);
	EXPECT_EQ(gtao_samples(-1).slices, 0);
}

TEST_F(Gtao, PixelRadiusFormula)
{
	// proj11 = 2 and 720 lines: 0.03 * 2 * 360 = 21.6 px
	EXPECT_FLOAT_EQ(gtao_pixel_radius(1.0f, 2.0f, 720), 21.6f);
	EXPECT_FLOAT_EQ(gtao_pixel_radius(2.0f, 2.0f, 720), 43.2f);
}

TEST_F(Gtao, PixelRadiusScalesWithResolution)
{
	EXPECT_FLOAT_EQ(gtao_pixel_radius(1.0f, 2.0f, 1440), 2.0f * gtao_pixel_radius(1.0f, 2.0f, 720));
}

TEST_F(Gtao, PixelRadiusIsClamped)
{
	// Upper limit: a quarter of the screen height
	EXPECT_FLOAT_EQ(gtao_pixel_radius(100.0f, 2.0f, 720), 180.0f);
	// The largest radius setting stays below the limit at proj11 = 2 (a 53 degree vertical field of view)
	EXPECT_LT(gtao_pixel_radius(6.0f, 2.0f, 720), 180.0f);
	// Lower limit: 2 px, so that a sample never lands on the center texel only
	EXPECT_FLOAT_EQ(gtao_pixel_radius(0.001f, 2.0f, 720), 2.0f);
}

TEST_F(Gtao, OptionsAreRegisteredWithDefaults)
{
	auto quality = typed_option<int>("Graphics.SsaoQuality");
	auto radius = typed_option<float>("Graphics.SsaoRadius");
	auto intensity = typed_option<float>("Graphics.SsaoIntensity");
	ASSERT_NE(quality, nullptr);
	ASSERT_NE(radius, nullptr);
	ASSERT_NE(intensity, nullptr);

	// Off by default, so the default render does not change
	EXPECT_EQ(quality->getDefaultValueFunc()(), 0);
	EXPECT_FLOAT_EQ(radius->getDefaultValueFunc()(), 1.0f);
	EXPECT_FLOAT_EQ(intensity->getDefaultValueFunc()(), 2.2f);
	// The lab sliders use the same constants (GTAO_*_MIN/MAX in gtao.h).
	EXPECT_EQ(radius->getRangeValues(), std::make_pair(0.25f, 6.0f));
	EXPECT_EQ(intensity->getRangeValues(), std::make_pair(0.5f, 5.0f));
}

// The options menu greys an option out when gr_option_requirement_met() says no for the running renderer.
TEST_F(Gtao, OptionsAreGreyedOutOnVulkanOnly)
{
	gtao_set_option_requirements();

	auto* mgr = OptionsManager::instance();
	for (auto key : {"Graphics.SsaoQuality", "Graphics.SsaoRadius", "Graphics.SsaoIntensity"}) {
		auto requirement = mgr->getRequirement(key);
		ASSERT_TRUE(requirement.has_value()) << key;
		EXPECT_FALSE(gr_option_requirement_met(requirement->first, GraphicsAPI::Vulkan, true)) << key;
		EXPECT_TRUE(gr_option_requirement_met(requirement->first, GraphicsAPI::OpenGL, false)) << key;
	}
}

TEST_F(Gtao, OnlyOpenGLRunsThePass)
{
	EXPECT_TRUE(gtao_supported(GraphicsAPI::OpenGL));
	EXPECT_FALSE(gtao_supported(GraphicsAPI::Vulkan));
	EXPECT_FALSE(gtao_supported(GraphicsAPI::Stub));
}

// The deferred cockpit pass clears the position buffer to 0, not to the empty-pixel value,
// and would also pay for the pass twice per frame. So only the scene pass runs it.
TEST_F(Gtao, RunsOnlyInTheScenePassOnOpenGL)
{
	EXPECT_TRUE(gtao_runs(2, GraphicsAPI::OpenGL, false));
	EXPECT_FALSE(gtao_runs(2, GraphicsAPI::OpenGL, true));
	EXPECT_FALSE(gtao_runs(0, GraphicsAPI::OpenGL, false));
	EXPECT_FALSE(gtao_runs(2, GraphicsAPI::Vulkan, false));
}
