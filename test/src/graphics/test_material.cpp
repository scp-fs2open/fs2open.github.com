#include <gtest/gtest.h>
#include <graphics/material.h>

// USE_NORMAL_MAP means "read the scene normal buffer": set when the decal brings no normal map of its own.
TEST(DecalMaterial, SceneNormalFlagSetOnlyWithoutDecalNormalMap)
{
	decal_material without_normal;
	without_normal.set_texture_map(TM_BASE_TYPE, 0);
	without_normal.set_texture_map(TM_NORMAL_TYPE, -1);
	EXPECT_EQ(without_normal.get_shader_flags(), static_cast<uint>(SDR_FLAG_DECAL_USE_NORMAL_MAP));

	decal_material with_normal;
	with_normal.set_texture_map(TM_BASE_TYPE, 0);
	with_normal.set_texture_map(TM_NORMAL_TYPE, 1);
	EXPECT_EQ(with_normal.get_shader_flags(), 0u);
}
