#include "graphics/post_processing.h"
#include "util/FSTestFixture.h"

#include <gtest/gtest.h>

class LightshaftParamsTest : public test::FSTestFixture {
  public:
	LightshaftParamsTest() : test::FSTestFixture(INIT_CFILE)
	{
		pushModDir("graphics/lightshafts_cockpit");
	}
};

// set-post-effect "lightshafts" only rewrites intensity, so the cockpit weight must not have it baked in
TEST_F(LightshaftParamsTest, CockpitWeightLeavesIntensityToDrawTime)
{
	graphics::PostProcessingManager manager;
	ASSERT_TRUE(manager.parse_table());

	const auto& params = manager.getLightshaftParams();
	ASSERT_FLOAT_EQ(params.intensity, 0.3f);
	EXPECT_NEAR(params.cpweight, 0.1f * (1.0f + 0.9f + 0.81f + 0.729f), 1e-6f);
}
