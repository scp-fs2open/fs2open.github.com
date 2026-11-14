#include "graphics/grinternal.h"
#include "graphics/post_processing.h"
#include "graphics/vulkan/VulkanPostProcessing.h"
#include "util/FSTestFixture.h"

#include <gtest/gtest.h>

class VulkanPostProcessResetTest : public test::FSTestFixture {
  public:
	VulkanPostProcessResetTest() : test::FSTestFixture(INIT_CFILE)
	{
		pushModDir("graphics/post_processing_reset");
	}

  protected:
	void TearDown() override
	{
		graphics::Post_processing_manager = nullptr;
		test::FSTestFixture::TearDown();
	}
};

// The Vulkan reset needs no renderer, unlike the OpenGL one, so it is the one a test can drive
TEST_F(VulkanPostProcessResetTest, SetDefaultsRestoresTheTableValues)
{
	graphics::Post_processing_manager.reset(new graphics::PostProcessingManager());
	ASSERT_TRUE(graphics::Post_processing_manager->parse_table());

	auto& effects = graphics::Post_processing_manager->getPostEffects();
	ASSERT_FALSE(effects.empty());
	for (auto& effect : effects) {
		effect.intensity = effect.default_intensity + 5.0f;
		effect.rgb = vm_vec_new(1.0f, 0.0f, 0.0f);
	}
	auto& params = graphics::Post_processing_manager->getLightshaftParams();
	params.intensity = 0.0f;
	params.on = false;

	graphics::vulkan::vulkan_post_process_set_defaults();

	for (const auto& effect : effects) {
		EXPECT_FLOAT_EQ(effect.intensity, effect.default_intensity) << effect.name;
		EXPECT_TRUE(effect.rgb == effect.default_rgb) << effect.name;
	}
	EXPECT_FLOAT_EQ(params.intensity, 0.3f);
	EXPECT_TRUE(params.on);
}
