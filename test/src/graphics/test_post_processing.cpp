#include "graphics/post_processing.h"
#include "util/FSTestFixture.h"

#include <gtest/gtest.h>

class PostProcessingResetTest : public test::FSTestFixture {
  public:
	PostProcessingResetTest() : test::FSTestFixture(INIT_CFILE)
	{
		pushModDir("graphics/post_processing_reset");
	}
};

TEST_F(PostProcessingResetTest, ResetRestoresTheTableLightshafts)
{
	graphics::PostProcessingManager manager;
	ASSERT_TRUE(manager.parse_table());

	auto& params = manager.getLightshaftParams();
	ASSERT_FLOAT_EQ(params.intensity, 0.3f);
	ASSERT_TRUE(params.on);

	params.intensity = 0.0f;
	params.on = false;
	manager.resetLightshaftParams();

	EXPECT_FLOAT_EQ(params.intensity, 0.3f);
	EXPECT_TRUE(params.on);
	EXPECT_FLOAT_EQ(params.density, 0.6f);
	EXPECT_EQ(params.samplenum, 40);
}

TEST_F(PostProcessingResetTest, EachEffectKeepsItsTableColor)
{
	graphics::PostProcessingManager manager;
	ASSERT_TRUE(manager.parse_table());

	const auto& effects = manager.getPostEffects();
	ASSERT_EQ(effects.size(), 2u);
	for (const auto& effect : effects) {
		if (effect.name == "tint") {
			EXPECT_FLOAT_EQ(effect.default_rgb.xyz.x, 0.1f);
			EXPECT_FLOAT_EQ(effect.default_rgb.xyz.y, 0.4f);
			EXPECT_FLOAT_EQ(effect.default_rgb.xyz.z, 0.7f);
		} else {
			EXPECT_TRUE(effect.default_rgb == vmd_zero_vector);
		}
	}
}
