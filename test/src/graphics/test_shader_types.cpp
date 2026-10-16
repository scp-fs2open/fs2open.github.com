#include <gtest/gtest.h>
#include <graphics/shader_types.h>

// decal-v.sdr reads its per-instance matrix from locations 8-11; a mask that only
// sets bit 8 makes the Vulkan pipeline drop the other three columns, so every
// decal is transformed by garbage and none reaches the screen (#7866).
TEST(ShaderTypes, ModelMatrixInputMaskCoversFourLocations)
{
	const ShaderTypeInfo* decal = shader_get_type_info(SDR_TYPE_DECAL);
	ASSERT_NE(decal, nullptr);

	const uint32_t mask = shader_vertex_input_mask(*decal);
	for (uint32_t loc = VATTRIB_MODEL_MATRIX; loc < VATTRIB_MODEL_MATRIX + 4; ++loc) {
		EXPECT_TRUE(mask & (1u << loc)) << "location " << loc << " missing from mask " << mask;
	}
	EXPECT_TRUE(mask & (1u << VATTRIB_POSITION));
}

TEST(ShaderTypes, ScalarAttributesSetOneBitEach)
{
	const ShaderTypeInfo* nanovg = shader_get_type_info(SDR_TYPE_NANOVG);
	ASSERT_NE(nanovg, nullptr);

	EXPECT_EQ(shader_vertex_input_mask(*nanovg), (1u << VATTRIB_POSITION) | (1u << VATTRIB_TEXCOORD));
}
