#include <gtest/gtest.h>
#include <graphics/shader_types.h>

// A mat4 input spans locations 8-11; the Vulkan pipeline drops any location the mask omits.
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

TEST(ShaderTypes, VertexInputMaskIsExactPerType)
{
	struct Case {
		shader_type type;
		uint32_t expected;
	};
	const Case cases[] = {
		{SDR_TYPE_DECAL, (1u << VATTRIB_POSITION) | (0xFu << VATTRIB_MODEL_MATRIX)},
		{SDR_TYPE_NANOVG, (1u << VATTRIB_POSITION) | (1u << VATTRIB_TEXCOORD)},
		{SDR_TYPE_MODEL,
			(1u << VATTRIB_POSITION) | (1u << VATTRIB_TEXCOORD) | (1u << VATTRIB_NORMAL) | (1u << VATTRIB_TANGENT) |
				(1u << VATTRIB_MODELID)},
		{SDR_TYPE_EFFECT_PARTICLE,
			(1u << VATTRIB_POSITION) | (1u << VATTRIB_TEXCOORD) | (1u << VATTRIB_RADIUS) | (1u << VATTRIB_COLOR)},
	};
	for (const auto& c : cases) {
		const ShaderTypeInfo* info = shader_get_type_info(c.type);
		ASSERT_NE(info, nullptr) << "type " << static_cast<int>(c.type);
		EXPECT_EQ(shader_vertex_input_mask(*info), c.expected) << "type " << static_cast<int>(c.type);
	}
}
